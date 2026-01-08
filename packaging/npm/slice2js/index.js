//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

var spawn = require("child_process").spawn;
var path = require("path");
var os = require("os");

var platformPrefix = "";
var archPrefix = os.arch();
var exe = "";

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
        console.error("Unsupported platform: " + os.platform());
        process.exit(1);
}

var sliceDir = path.resolve(path.join(__dirname, "slice"));
var binDir = path.resolve(path.join(__dirname, "bin", platformPrefix + "-" + archPrefix));
var slice2js = path.resolve(path.join(binDir, exe));

/**
 * Compile Slice files to JavaScript.
 * @param {string[]} args - Command-line arguments for slice2js.
 * @param {object} options - Options passed to spawn.
 * @returns {ChildProcess} The spawned child process.
 */
function compile(args, options) {
    args = args || [];
    var slice2jsArgs = args.slice();
    slice2jsArgs.push("-I" + sliceDir);
    return spawn(slice2js, slice2jsArgs, options);
}

module.exports.compile = compile;
module.exports.sliceDir = sliceDir;
module.exports.slice2js = slice2js;
