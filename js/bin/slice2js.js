#!/usr/bin/env node

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
 * @param {object} options - Options passed to spawn.
 * @returns {Promise<number>} Resolves with the exit code.
 */
export async function compile(args = [], options = {}) {
    if (!fs.existsSync(slice2js)) {
        throw new Error(`Missing slice2js executable at ${slice2js}`);
    }

    let slice2jsArgs = args.slice();
    slice2jsArgs.push(`-I${slicePath}`);

    return new Promise((resolve, reject) => {
        const process = spawn(slice2js, slice2jsArgs, options);

        process.on("error", reject);
        process.on("exit", resolve);
    });
}

// If executed directly via `npx slice2js`
if (import.meta.url === `file://${process.argv[1]}`) {
    try {
        const exitCode = await compile(process.argv.slice(2), { stdio: "inherit" });
        process.exit(exitCode);
    } catch (err) {
        console.error(`Error executing slice2js: ${err.message}`);
        process.exit(1);
    }
}

// Options for configuring gulp-ice-builder to use this package
const builderOptions = {
    iceHome: path.resolve(__dirname, ".."),
    iceToolsPath: binDir,
}

export { slicePath, slice2js, builderOptions };
