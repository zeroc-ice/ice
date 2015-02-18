// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var gulp = require('gulp');
var path = require('path');

var libTasks = require('./gulp/libTasks')(gulp);

gulp.task('build', libTasks.buildTasks);
gulp.task('clean', libTasks.cleanTasks);
gulp.task('watch', libTasks.watchTasks);
