// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;

module.exports.Ice = _ModuleRegistry.require(module,
    [
        "../Ice/Initialize",
        "../Ice/Communicator",
        "../Ice/CommunicatorI",
        "../Ice/HashMap",
        "../Ice/Object",
        "../Ice/Value",
        "../Ice/Long",
        "../Ice/Logger",
        "../Ice/LoggerI",
        "../Ice/ObjectPrx",
        "../Ice/BatchRequestQueue",
        "../Ice/Properties",
        "../Ice/PropertiesI",
        "../Ice/IdentityUtil",
        "../Ice/ProcessLogger",
        "../Ice/Protocol",
        "../Ice/Identity",
        "../Ice/Exception",
        "../Ice/LocalException",
        "../Ice/BuiltinSequences",
        "../Ice/StreamHelpers",
        "../Ice/Promise",
        "../Ice/EndpointTypes",
        "../Ice/Locator",
        "../Ice/Router",
        "../Ice/Version",
        "../Ice/Buffer",
        "../Ice/ArrayUtil",
        "../Ice/UnknownSlicedValue",
        "../Ice/Process",
        "../Ice/MapUtil",
        "../Ice/ToStringMode"
    ]).Ice;

module.exports.IceMX = require("../Ice/Metrics").IceMX;

module.exports.IceSSL = require("../Ice/EndpointInfo").IceSSL;
