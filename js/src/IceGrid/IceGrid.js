// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;

module.exports.IceGrid = _ModuleRegistry.require(module,
    [
        "../IceGrid/Admin",
        "../IceGrid/Descriptor",
        "../IceGrid/Exception",
        "../IceGrid/FileParser",
        "../IceGrid/Registry",
        "../IceGrid/Session",
        "../IceGrid/UserAccountMapper"
    ]).IceGrid;
