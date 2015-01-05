#!/usr/bin/env node
// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

'use strict';

var binPath = require('../installSlice2js').path;
var spawn = require('child_process').spawn;
var path = require('path');

var SLICE_DIR = path.resolve(path.join('..', 'slice'));

var args = process.argv.slice(2);
args.push('-I'+SLICE_DIR);

spawn(binPath, args, { stdio: 'inherit' }).on('exit', process.exit);
