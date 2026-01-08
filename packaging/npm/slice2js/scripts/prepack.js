//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

var fs = require("fs");
var os = require("os");
var path = require("path");

var platformPrefix = "";
var archPrefix = "";
var slice2js = "";

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
        console.error("Unsupported platform: " + os.platform());
        process.exit(1);
}

var allSupportedPlatforms = ["windows-x64", "macos-arm64", "linux-x64", "linux-arm64"];

var sliceSourceDir = path.resolve(__dirname, "../../../../slice");
var sliceDestDir = path.resolve(__dirname, "../slice");
var cppBinDir = path.resolve(__dirname, "../../../../cpp/bin");

if (os.platform() === "win32") {
    cppBinDir = path.resolve(cppBinDir, "x64", "Release");
}

function copyFolderSync(src, dest) {
    fs.mkdirSync(dest, { recursive: true });
    fs.readdirSync(src).forEach(function (file) {
        var srcFile = path.join(src, file);
        var destFile = path.join(dest, file);

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
    console.error("See https://github.com/zeroc-ice/ice");
    process.exit(1);
}

console.log("Copying Slice files from " + sliceSourceDir + " to " + sliceDestDir + "...");
copyFolderSync(sliceSourceDir, sliceDestDir);
console.log("Done.");

var stagingPath = process.env.SLICE2JS_STAGING_PATH;
var binDestDir = path.resolve(__dirname, "../bin");

if (stagingPath) {
    // Check that all compilers are available in the staging path
    for (var i = 0; i < allSupportedPlatforms.length; i++) {
        var platform = allSupportedPlatforms[i];
        var compilerPath = path.join(
            stagingPath,
            platform,
            platform.startsWith("windows") ? "slice2js.exe" : "slice2js"
        );
        if (!fs.existsSync(compilerPath)) {
            console.error("Missing compiler " + compilerPath + " in the staging path " + stagingPath);
            process.exit(1);
        }
    }

    console.log("Copying Slice compilers from " + stagingPath + " to " + binDestDir + "...");
    copyFolderSync(stagingPath, binDestDir);
    console.log("Done.");
} else {
    console.log("Copying Slice compiler from cpp build...");
    var compilerPath = path.join(cppBinDir, slice2js);
    if (!fs.existsSync(compilerPath)) {
        console.error("Missing compiler " + compilerPath + " in the source distribution");
        console.error("The slice2js compiler must be built before running the pack command.");
        process.exit(1);
    }

    var platformBinDir = path.join(binDestDir, platformPrefix + "-" + archPrefix);
    fs.mkdirSync(platformBinDir, { recursive: true });
    fs.copyFileSync(compilerPath, path.join(platformBinDir, slice2js));
    console.log("Done.");
}
