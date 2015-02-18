// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var path = require('path');
var gulpSlice2js = require('gulp-zeroc-slice2js');
var sliceDir = gulpSlice2js.sliceDir;

function slice2js(options) {
    return gulpSlice2js.compile(options);
}

module.exports.slice2js = slice2js;
module.exports.sliceDir = sliceDir;
