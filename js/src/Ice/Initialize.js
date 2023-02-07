//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Communicator");
require("../Ice/LocalException");
require("../Ice/Properties");
require("../Ice/Protocol");

const Protocol = Ice.Protocol;

//
// Ice.InitializationData
//
Ice.InitializationData = class
{
    constructor()
    {
        this.properties = null;
        this.logger = null;
        this.valueFactoryManager = null;
    }

    clone()
    {
        const r = new Ice.InitializationData();
        r.properties = this.properties;
        r.logger = this.logger;
        r.valueFactoryManager = this.valueFactoryManager;
        return r;
    }
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
    return "3.7.9"; // "A.B.C", with A=major, B=minor, C=patch
};

Ice.intVersion = function()
{
    return 30709; // AABBCC, with AA=major, BB=minor, CC=patch
};

module.exports.Ice = Ice;
