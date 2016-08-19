// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/TcpEndpointI").Ice;
const TcpEndpointI = Ice.TcpEndpointI;

class TcpEndpointFactory
{
    constructor(instance)
    {
        this._instance = instance;
    }

    type()
    {
        return this._instance.type();
    }

    protocol()
    {
        return this._instance.protocol();
    }

    create(args, oaEndpoint)
    {
        const e = new TcpEndpointI(this._instance);
        e.initWithOptions(args, oaEndpoint);
        return e;
    }

    read(s)
    {
        const e = new TcpEndpointI(this._instance);
        e.initWithStream(s);
        return e;
    }

    destroy()
    {
        this._instance = null;
    }

    clone(instance)
    {
        return new TcpEndpointFactory(instance);
    }
}

Ice.TcpEndpointFactory = TcpEndpointFactory;
module.exports.Ice = Ice;
