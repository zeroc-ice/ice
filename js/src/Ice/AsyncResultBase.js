// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/Debug", "../Ice/Promise", "../Ice/Exception"]);

var Promise = Ice.Promise;

var AsyncResultBase = Ice.Class(Promise, {
    __init__: function(communicator, op, connection, proxy, adapter)
    {
        //
        // AsyncResultBase can be constructed by a sub-type's prototype, in which case the
        // arguments are undefined.
        //
        Promise.call(this);
        if(communicator !== undefined)
        {
            this._communicator = communicator;
            this._instance = communicator !== null ? communicator.instance : null;
            this._operation = op;
            this._connection = connection;
            this._proxy = proxy;
            this._adapter = adapter;
        }
    }
});

var prototype = AsyncResultBase.prototype;
var defineProperty = Object.defineProperty;

defineProperty(prototype, "communicator", {
    get: function() { return this._communicator; }
});

defineProperty(prototype, "connection", {
    get: function() { return this._connection; }
});

defineProperty(prototype, "proxy", {
    get: function() { return this._proxy; }
});

defineProperty(prototype, "adapter", {
    get: function() { return this._adapter; }
});

defineProperty(prototype, "operation", {
    get: function() { return this._operation; }
});

Ice.AsyncResultBase = AsyncResultBase;

module.exports.Ice = Ice;
