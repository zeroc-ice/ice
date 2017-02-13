// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice._ModuleRegistry.require(module,
    [
        "../Ice/Protocol",
        "../Ice/LocalException",
        "../Ice/Communicator",
        "../Ice/Properties"
    ]);

const Protocol = Ice.Protocol;

//
// Ice.InitializationData
//
Ice.InitializationData = function()
{
    this.properties = null;
    this.logger = null;
    this.valueFactoryManager = null;
};

Ice.InitializationData.prototype.clone = function()
{
    const r = new Ice.InitializationData();
    r.properties = this.properties;
    r.logger = this.logger;
    r.valueFactoryManager = this.valueFactoryManager;
    return r;
};

//
// Ice.initialize()
//
Ice.initialize = function(arg1, arg2)
{
    let args = null;
    let initData = null;

    if(arg1 instanceof Array)
    {
        args = arg1;
    }
    else if(arg1 instanceof Ice.InitializationData)
    {
        initData = arg1;
    }
    else if(arg1 !== undefined && arg1 !== null)
    {
        throw new Ice.InitializationException("invalid argument to initialize");
    }

    if(arg2 !== undefined && arg2 !== null)
    {
        if(arg2 instanceof Ice.InitializationData && initData === null)
        {
            initData = arg2;
        }
        else
        {
            throw new Ice.InitializationException("invalid argument to initialize");
        }
    }

    if(initData === null)
    {
        initData = new Ice.InitializationData();
    }
    else
    {
        initData = initData.clone();
    }
    initData.properties = Ice.createProperties(args, initData.properties);

    const result = new Ice.Communicator(initData);
    result.finishSetup(null);
    return result;
};

//
// Ice.createProperties()
//
Ice.createProperties = function(args, defaults)
{
    return new Ice.Properties(args, defaults);
};

Ice.currentProtocol = function()
{
    return Protocol.currentProtocol.clone();
};

Ice.currentEncoding = function()
{
    return Protocol.currentEncoding.clone();
};

Ice.stringVersion = function()
{
    return "3.7a4";
};

Ice.intVersion = function()
{
    return 30754;
};

module.exports.Ice = Ice;
