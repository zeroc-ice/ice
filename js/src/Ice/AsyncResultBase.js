//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { Promise } from './Promise.js';

export class AsyncResultBase extends Promise
{
    constructor(communicator, op, connection, proxy, adapter)
    {
        super();
        this._communicator = communicator;
        this._instance = communicator ? communicator.instance : null;
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
