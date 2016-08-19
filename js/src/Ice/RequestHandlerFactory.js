// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Debug",
        "../Ice/HashMap",
        "../Ice/Reference",
        "../Ice/ConnectRequestHandler"
    ]);

const Debug = Ice.Debug;
const HashMap = Ice.HashMap;
const ConnectRequestHandler = Ice.ConnectRequestHandler;

class RequestHandlerFactory
{
    constructor(instance)
    {
        this._instance = instance;
        this._handlers = new HashMap(HashMap.compareEquals);
    }

    getRequestHandler(ref, proxy)
    {
        let connect = false;
        let handler;
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
            ref.getConnection().then(values => 
                                     {
                                         handler.setConnection(values);
                                     },
                                     ex =>
                                     {
                                         handler.setException(ex);
                                     });
        }
        return proxy.__setRequestHandler(handler.connect(proxy));
    }

    removeRequestHandler(ref, handler)
    {
        if(ref.getCacheConnection())
        {
            if(this._handlers.get(ref) === handler)
            {
                this._handlers.delete(ref);
            }
        }
    }
}

Ice.RequestHandlerFactory = RequestHandlerFactory;
module.exports.Ice = Ice;
