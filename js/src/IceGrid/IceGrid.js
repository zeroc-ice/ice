//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;

require("../IceGrid/Admin");
require("../IceGrid/Descriptor");
require("../IceGrid/Exception");
require("../IceGrid/FileParser");
require("../IceGrid/Registry");
require("../IceGrid/Session");
require("../IceGrid/UserAccountMapper");

module.exports.IceGrid = _ModuleRegistry.module("IceGrid");
