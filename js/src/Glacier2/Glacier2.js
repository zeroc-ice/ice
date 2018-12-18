// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;

module.exports.Glacier2 = _ModuleRegistry.require(module,
    [
        "../Glacier2/PermissionsVerifier",
        "../Glacier2/Router",
        "../Glacier2/Session",
        "../Glacier2/SSLInfo"
    ]).Glacier2;
