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

var slice2js = require('../slice2js');
slice2js(process.argv.slice(2), {stdio: 'inherit'}).on('exit', process.exit);
