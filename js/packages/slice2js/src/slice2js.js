// Copyright (c) ZeroC, Inc.

import { spawn } from "child_process";
import path from "path";
import os from "os";
import { fileURLToPath } from "url";
import fs from "fs";

const __dirname = path.dirname(fileURLToPath(import.meta.url));

let platformPrefix = "";
let archPrefix = os.arch();
let exe = "";

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
        exe = "slice2js";
        break;
    default:
        console.error(`Unsupported platform: ${os.platform()}`);
        process.exit(1);
}

const slicePath = path.resolve(__dirname, "..", "slice");
const binDir = path.resolve(__dirname, "..", "bin", `${platformPrefix}-${archPrefix}`);
const slice2js = path.resolve(binDir, exe);

/**
 * Compile function for programmatic use.
 * @param {string[]} args - Command-line arguments for slice2js.
 * @param {object} [options]
 * @param {string} [options.toolsPath] - Directory containing the slice2js binary. Overrides the bundled binary.
 * @param {string} [options.slicePath] - Directory containing the Slice definitions. Overrides the bundled path.
 * @param {string} [options.cwd] - Working directory for the compiler process.
 * @returns {Promise<number>} Resolves with the exit code.
 */
export async function compile(args = [], { toolsPath, slicePath: customSlicePath, cwd } = {}) {
    const compiler = toolsPath ? path.resolve(toolsPath, exe) : slice2js;
    const sliceInc = customSlicePath || slicePath;

    if (!fs.existsSync(compiler)) {
        throw new Error(`Missing slice2js executable at ${compiler}`);
    }

    let slice2jsArgs = args.slice();
    slice2jsArgs.push(`-I${sliceInc}`);

    return new Promise((resolve, reject) => {
        const proc = spawn(compiler, slice2jsArgs, {
            stdio: ["ignore", "inherit", "inherit"],
            cwd,
        });
        proc.on("error", reject);
        proc.on("exit", resolve);
    });
}

export { slicePath, slice2js };
