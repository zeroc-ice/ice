// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var __M = require("../Ice/ModuleRegistry").Ice.__M;

module.exports.IceGrid = __M.require(module,
    [
        "../IceGrid/Admin",
        "../IceGrid/Descriptor",
        "../IceGrid/Exception",
        "../IceGrid/FileParser",
        "../IceGrid/Locator",
        "../IceGrid/Observer",
        "../IceGrid/Query",
        "../IceGrid/Registry",
        "../IceGrid/Session",
        "../IceGrid/UserAccountMapper"
    ]).IceGrid;
