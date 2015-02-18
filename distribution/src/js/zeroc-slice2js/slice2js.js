// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var spawn     = require('child_process').spawn;
var path      = require('path');
var os        = require('os');
var platform  = os.platform();
var arch      = os.arch();

var sliceDir = path.resolve(path.join(__dirname, 'slice'));
var slice2js = path.join(path.join(__dirname, 'build', 'Release'),
                                   platform === 'win32' ? 'slice2js.exe' : 'slice2js');

function compile(args, options)
{
    args = args || [];
    var slice2jsArgs = args.slice();
    slice2jsArgs.push('-I' + sliceDir);
    return spawn(slice2js, slice2jsArgs, options);
}

module.exports.compile  = compile;
module.exports.sliceDir = sliceDir;
