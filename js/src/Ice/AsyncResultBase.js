// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/Debug");
    require("Ice/Promise");
    require("Ice/Exception");

    var Ice = global.Ice || {};

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
        },
        __exception: function(ex)
        {
            this.fail(ex);
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

    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
