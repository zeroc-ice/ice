// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var __M = require("../Ice/ModuleRegistry").Ice.__M;

module.exports.Glacier2 = __M.require(module,
    [
        "../Glacier2/PermissionsVerifier",
        "../Glacier2/Router",
        "../Glacier2/Session",
        "../Glacier2/SSLInfo",
    ]).Glacier2;
