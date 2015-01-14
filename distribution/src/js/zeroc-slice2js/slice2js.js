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

module.exports = function(args, options)
{
  var bin_dir   = path.join(__dirname, 'build', 'Release');
  var slice2js  = platform === 'win32' ? 'slice2js.exe' : 'slice2js';
  slice2js      = path.join(bin_dir, slice2js);

  var slice_dir = path.resolve(path.join(__dirname, 'slice'));

  var slice2js_args = args.slice();
  slice2js_args.push('-I' + slice_dir);

  return spawn(slice2js, slice2js_args, options);
};
