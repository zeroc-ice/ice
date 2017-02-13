// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/Promise").Ice;

class AsyncResultBase extends Ice.Promise
{
    constructor(communicator, op, connection, proxy, adapter)
    {
        super();
        this._communicator = communicator;
        this._instance = communicator !== null ? communicator.instance : null;
        this._operation = op;
        this._connection = connection;
        this._proxy = proxy;
        this._adapter = adapter;
    }

    get communicator()
    {
        return this._communicator;
    }

    get connection()
    {
        return this._connection;
    }

    get proxy()
    {
        return this._proxy;
    }

    get adapter()
    {
        return this._adapter; 
    }

    get operation()
    {
        return this._operation;
    }
}

Ice.AsyncResultBase = AsyncResultBase;

module.exports.Ice = Ice;
