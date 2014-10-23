// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Debug",
        "../Ice/HashMap",
        "../Ice/Reference",
        "../Ice/ConnectRequestHandler"
    ]);

var Debug = Ice.Debug;
var HashMap = Ice.HashMap;
var ConnectRequestHandler = Ice.ConnectRequestHandler;

var RequestHandlerFactory = Ice.Class({
    __init__: function(instance)
    {
        this._instance = instance;
        this._handlers = new HashMap(HashMap.compareEquals);
    },
    getRequestHandler: function(ref, proxy)
    {
        if(ref.getCacheConnection())
        {
            var handler = this._handlers.get(ref);
            if(handler)
            {
                return handler;
            }
            handler = new ConnectRequestHandler(ref, proxy);
            this._handlers.set(ref, handler);
            return handler;
        }
        else
        {
            return new ConnectRequestHandler(ref, proxy);
        }
    },
    removeRequestHandler: function(ref, handler)
    {
        if(ref.getCacheConnection())
        {
            var h = this._handlers.delete(ref);
            Debug.assert(h === handler);
        }
    }
});

Ice.RequestHandlerFactory = RequestHandlerFactory;
module.exports.Ice = Ice;
