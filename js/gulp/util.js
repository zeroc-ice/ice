// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var path         = require('path');

var gulpSlice2js = require("./gulp-slice2js");
var sliceDir   = path.resolve(__dirname, '..', '..', 'slice');

var useBinDist = process.env.USE_BIN_DIST == "yes";

function getSliceArgs(options)
{
    var defaults = {};
    var opts = options || {};

    defaults.args = opts.args || [];
    defaults.dest = opts.dest;

    if(useBinDist)
    {
        defaults.exe = undefined;
    }
    else
    {
        defaults.args = defaults.args.concat(["-I" + sliceDir]);
        defaults.exe = opts.exe || path.resolve(
            path.join("../cpp/bin", process.platform == "win32" ? "slice2js.exe" : "slice2js"));
    }
    return defaults;
}

function slice2js(options) {
    return gulpSlice2js.compile(getSliceArgs(options));
}

module.exports.useBinDist = useBinDist;
module.exports.slice2js = slice2js;
module.exports.sliceDir = sliceDir;
