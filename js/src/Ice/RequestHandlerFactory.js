//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/ConnectRequestHandler");
require("../Ice/HashMap");
require("../Ice/Reference");

const ConnectRequestHandler = Ice.ConnectRequestHandler;
const HashMap = Ice.HashMap;

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
            ref.getConnection().then(connection =>
                                     {
                                         handler.setConnection(connection);
                                     },
                                     ex =>
                                     {
                                         handler.setException(ex);
                                     });
        }
        return proxy._setRequestHandler(handler.connect(proxy));
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
