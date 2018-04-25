// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/WSEndpoint").Ice;
const WSEndpoint = Ice.WSEndpoint;

class WSEndpointFactory extends WSEndpoint
{
    constructor(instance, delegate)
    {
        super();
        this._instance = instance;
        this._delegate = delegate;
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
        const e = new WSEndpoint(this._instance, this._delegate.create(args, oaEndpoint));
        e.initWithOptions(args);
        return e;
    }

    read(s)
    {
        const e = new WSEndpoint(this._instance, this._delegate.read(s));
        e.initWithStream(s);
        return e;
    }

    destroy()
    {
        this._delegate.destroy();
        this._instance = null;
    }
}

Ice.WSEndpointFactory = WSEndpointFactory;
exports.Ice = Ice;
