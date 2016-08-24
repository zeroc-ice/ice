// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.Callable;

public final class RequestHandlerFactory
{
    RequestHandlerFactory(Instance instance)
    {
        _instance = instance;
    }

    public RequestHandler
    getRequestHandler(final RoutableReference ref, com.zeroc.Ice._ObjectPrxI proxy)
    {
        if(ref.getCollocationOptimized())
        {
            com.zeroc.Ice.ObjectAdapter adapter = _instance.objectAdapterFactory().findObjectAdapter(proxy);
            if(adapter != null)
            {
                return proxy.__setRequestHandler(new CollocatedRequestHandler(ref, adapter));
            }
        }

        ConnectRequestHandler handler = null;
        boolean connect = false;
        if(ref.getCacheConnection())
        {
            synchronized(this)
            {
                handler = _handlers.get(ref);
                if(handler == null)
                {
                    handler = new ConnectRequestHandler(ref, proxy);
                    _handlers.put(ref, handler);
                    connect = true;
                }
            }
        }
        else
        {
            handler = new ConnectRequestHandler(ref, proxy);
            connect = true;
        }

        if(connect)
        {
            if(_instance.queueRequests())
            {
                final ConnectRequestHandler h = handler;
                _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
                                                            {
                                                                @Override
                                                                public Void call()
                                                                {
                                                                    ref.getConnection(h);
                                                                    return null;
                                                                }
                                                            });
            }
            else
            {
                ref.getConnection(handler);
            }
        }
        return proxy.__setRequestHandler(handler.connect(proxy));
    }

    void
    removeRequestHandler(Reference ref, RequestHandler handler)
    {
        if(ref.getCacheConnection())
        {
            synchronized(this)
            {
                if(_handlers.get(ref) == handler)
                {
                    _handlers.remove(ref);
                }
            }
        }
    }

    private final Instance _instance;
    private final Map<Reference, ConnectRequestHandler> _handlers = new HashMap<>();
}
