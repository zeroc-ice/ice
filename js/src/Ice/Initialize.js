// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Protocol",
        "../Ice/LocalException",
        "../Ice/Communicator",
        "../Ice/Properties"
    ]);

var Protocol = Ice.Protocol;

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
    var r = new Ice.InitializationData();
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
    var args = null;
    var initData = null;

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

    var result = new Ice.Communicator(initData);
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

module.exports.Ice = Ice;
