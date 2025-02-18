// Copyright (c) ZeroC, Inc.

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

const sliceSourceDir = path.resolve(__dirname, "../../slice");
const sliceDestDir = path.resolve(__dirname, "../slice");
let cppBinDir = path.resolve(__dirname, "../../cpp/bin");

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

// Ensure we are running from Ice source distribution
if (!fs.existsSync(sliceSourceDir)) {
    console.error("The pack command must be run from the Ice source distribution.");
    console.error("See https://github.com/zeroc-ice/ice/blob/main/js/BUILDING.md");
    process.exit(1);
}

console.log(`Copying Slice files from ${sliceSourceDir} to ${sliceDestDir}... `);
copyFolderSync(sliceSourceDir, sliceDestDir);
console.log("Done.");

const stagingPath = process.env.SLICE2JS_STAGING_PATH;
let binDestDir = path.resolve(__dirname, "../bin");

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
