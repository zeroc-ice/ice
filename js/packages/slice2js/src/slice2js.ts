// Copyright (c) ZeroC, Inc.

import { spawn } from "node:child_process";
import path from "node:path";
import os from "node:os";
import { fileURLToPath } from "node:url";
import fs from "node:fs";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

let platformPrefix: string;
let archPrefix: string;
let exe: string;

switch (os.platform()) {
    case "win32":
        platformPrefix = "windows";
        archPrefix = "x64";
        exe = "slice2js.exe";
        break;
    case "darwin":
        platformPrefix = "macos";
        archPrefix = "arm64";
        exe = "slice2js";
        break;
    case "linux":
        platformPrefix = "linux";
        archPrefix = os.arch();
        exe = "slice2js";
        break;
    default:
        throw new Error(`Unsupported platform: ${os.platform()}`);
}

const slicePath: string = path.resolve(__dirname, "..", "slice");
const binDir = path.resolve(__dirname, "..", "bin", `${platformPrefix}-${archPrefix}`);
const slice2js: string = path.resolve(binDir, exe);

export interface CompileOptions {
    /** Directory containing the slice2js binary. Overrides the bundled binary. */
    toolsPath?: string;
    /** Directory containing the Slice definitions. Overrides the bundled path. */
    slicePath?: string;
    /** Working directory for the compiler process. */
    cwd?: string;
}

/**
 * Compile Slice files using the slice2js compiler.
 * @param args - Command-line arguments for slice2js.
 * @param options - Optional configuration.
 * @returns Resolves with the exit code.
 */
export async function compile(
    args: string[] = [],
    { toolsPath, slicePath: customSlicePath, cwd }: CompileOptions = {},
): Promise<number> {
    const compiler = toolsPath ? path.resolve(toolsPath, exe) : slice2js;
    const sliceInc = customSlicePath || slicePath;

    if (!fs.existsSync(compiler)) {
        throw new Error(`Missing slice2js executable at ${compiler}`);
    }

    const slice2jsArgs = args.slice();
    slice2jsArgs.push(`-I${sliceInc}`);

    return new Promise((resolve, reject) => {
        const proc = spawn(compiler, slice2jsArgs, {
            stdio: ["ignore", "inherit", "inherit"],
            cwd,
        });
        proc.on("error", reject);
        proc.on("exit", (code) => resolve(code ?? 1));
    });
}

export { slicePath, slice2js };
