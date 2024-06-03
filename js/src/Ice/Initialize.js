//
// Copyright (c) ZeroC, Inc. All rights reserved.
//


import { Communicator } from './Communicator';
import { Protocol } from './Protocol';
import { InitializationException } from './LocalException';

//
// Ice.InitializationData
//
class InitializationData
{
    constructor()
    {
        this.properties = null;
        this.logger = null;
        this.valueFactoryManager = null;
    }

    clone()
    {
        const r = new InitializationData();
        r.properties = this.properties;
        r.logger = this.logger;
        r.valueFactoryManager = this.valueFactoryManager;
        return r;
    }
};

//
// Ice.initialize()
//
function initialize(arg1, arg2)
{
    let args = null;
    let initData = null;

    if(arg1 instanceof Array)
    {
        args = arg1;
    }
    else if(arg1 instanceof InitializationData)
    {
        initData = arg1;
    }
    else if(arg1 !== undefined && arg1 !== null)
    {
        throw new InitializationException("invalid argument to initialize");
    }

    if(arg2 !== undefined && arg2 !== null)
    {
        if(arg2 instanceof InitializationData && initData === null)
        {
            initData = arg2;
        }
        else
        {
            throw new InitializationException("invalid argument to initialize");
        }
    }

    if(initData === null)
    {
        initData = new InitializationData();
    }
    else
    {
        initData = initData.clone();
    }
    initData.properties = createProperties(args, initData.properties);

    const result = new Communicator(initData);
    result.finishSetup(null);
    return result;
};

//
// Ice.createProperties()
//
function createProperties(args, defaults)
{
    return new Ice.Properties(args, defaults);
}

function currentProtocol()
{
    return Protocol.currentProtocol.clone();
}

function currentEncoding()
{
    return Protocol.currentEncoding.clone();
};

function stringVersion()
{
    return "3.8.0-alpha.0"; // "A.B.C", with A=major, B=minor, C=patch
}

function intVersion()
{
    return 30850; // AABBCC, with AA=major, BB=minor, CC=patch
};
