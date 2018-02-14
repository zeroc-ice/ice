// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        var connect = false;
        var handler;
        if(ref.getCacheConnection())
        {
            handler = this._handlers.get(ref);
            if(!handler)
            {
                handler = new ConnectRequestHandler(ref, proxy);
                this._handlers.set(ref, handler);
                connect = true;
            }
        }
        else
        {
            connect = true;
            handler = new ConnectRequestHandler(ref, proxy);
        }

        if(connect)
        {
            ref.getConnection().then(function(connection, compress)
                                     {
                                         handler.setConnection(connection, compress);
                                     },
                                     function(ex)
                                     {
                                         handler.setException(ex);
                                     });
        }
        return proxy.__setRequestHandler(handler.connect(proxy));
    },
    removeRequestHandler: function(ref, handler)
    {
        if(ref.getCacheConnection())
        {
            if(this._handlers.get(ref) === handler)
            {
                this._handlers.delete(ref);
            }
        }
    }
});

Ice.RequestHandlerFactory = RequestHandlerFactory;
module.exports.Ice = Ice;
