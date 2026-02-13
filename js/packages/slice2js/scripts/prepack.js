// Copyright (c) ZeroC, Inc.

// Prepack script for @zeroc/slice2js. Bundles the Slice definitions and platform-specific slice2js compiler
// binaries into the package directory before packing. When SLICE2JS_STAGING_PATH is set, all supported
// platform binaries are copied from the staging directory; otherwise, the locally built binary is used.

import fs from "fs";
import os from "os";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let platformPrefix = "";
let archPrefix = "";
let slice2js = "";

switch (os.platform()) {
    case "win32":
        platformPrefix = "windows";
        archPrefix = "x64";
        slice2js = "slice2js.exe";
        break;
    case "darwin":
        platformPrefix = "macos";
        archPrefix = "arm64";
        slice2js = "slice2js";
        break;
    case "linux":
        platformPrefix = "linux";
        archPrefix = os.arch();
        slice2js = "slice2js";
        break;
    default:
        console.error(`Unsupported platform: ${os.platform()}`);
        process.exit(1);
}

const allSupportedPlatforms = ["windows-x64", "macos-arm64", "linux-x64", "linux-arm64"];

// Resolve paths relative to the Ice source distribution root.
// From js/packages/slice2js/scripts/ -> repo root is four levels up.
const iceHome = path.resolve(__dirname, "../../../..");
const sliceSourceDir = path.resolve(iceHome, "slice");
const sliceDestDir = path.resolve(__dirname, "../slice");
let cppBinDir = path.resolve(iceHome, "cpp/bin");

if (os.platform() === "win32") {
    cppBinDir = path.resolve(cppBinDir, "x64", "Release");
}

function copyFolderSync(src, dest) {
    fs.mkdirSync(dest, { recursive: true });
    fs.readdirSync(src).forEach(file => {
        const srcFile = path.join(src, file);
        const destFile = path.join(dest, file);

        if (fs.lstatSync(srcFile).isDirectory()) {
            copyFolderSync(srcFile, destFile);
        } else {
            fs.copyFileSync(srcFile, destFile);
        }
    });
}

if (!fs.existsSync(sliceSourceDir)) {
    console.error(`Slice source directory not found: ${sliceSourceDir}`);
    console.error("This script must be run from an Ice source distribution.");
    process.exit(1);
}
console.log(`Copying Slice files from ${sliceSourceDir} to ${sliceDestDir}... `);
copyFolderSync(sliceSourceDir, sliceDestDir);
console.log("Done.");

const stagingPath = process.env.SLICE2JS_STAGING_PATH;
let binDestDir = path.resolve(__dirname, "../bin");

// Copy hand-written type declarations for unplugin adapters.
// These replace the auto-generated ones to avoid leaking all bundler dependencies via unplugin's barrel types.
// See https://github.com/zeroc-ice/ice/issues/5067
const typesSourceDir = path.resolve(__dirname, "../types/unplugin");
const typesDestDir = path.resolve(__dirname, "../dist/unplugin");

console.log(`Copying unplugin type declarations from ${typesSourceDir} to ${typesDestDir}...`);
fs.readdirSync(typesSourceDir).forEach(file => {
    if (file.endsWith(".d.ts")) {
        fs.copyFileSync(path.join(typesSourceDir, file), path.join(typesDestDir, file));
    }
});
console.log("Done.");

if (stagingPath) {
    // Check that all compilers are available in the staging path
    for (const platform of allSupportedPlatforms) {
        const compilerPath = path.join(
            stagingPath,
            platform,
            platform.startsWith("windows") ? "slice2js.exe" : "slice2js",
        );
        if (!fs.existsSync(compilerPath)) {
            console.error(`Missing compiler ${compilerPath} in the staging path ${stagingPath}`);
            process.exit(1);
        }
    }

    console.log(`Copying Slice compilers from ${stagingPath} to ${binDestDir}...`);
    copyFolderSync(stagingPath, binDestDir);
    console.log("Done.");
} else {
    console.log(`Compiling Slice compiler from cpp build... `);
    const compilerPath = path.join(cppBinDir, slice2js);
    if (!fs.existsSync(compilerPath)) {
        console.error(`Missing compiler ${compilerPath} in the source distribution`);
        console.error(`The slice2js compiler must be built before running the pack command.`);
        process.exit(1);
    }

    fs.mkdirSync(path.join(binDestDir, `${platformPrefix}-${archPrefix}`), { recursive: true });
    fs.copyFileSync(compilerPath, path.join(binDestDir, `${platformPrefix}-${archPrefix}`, slice2js));
    console.log("Done.");
}
