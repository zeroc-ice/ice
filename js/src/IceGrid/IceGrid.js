//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;

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
