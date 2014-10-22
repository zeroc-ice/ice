// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.Map;
import java.util.HashMap;

public final class RequestHandlerFactory
{
    RequestHandlerFactory(Instance instance)
    {
        _instance = instance;
    }

    public RequestHandler 
    getRequestHandler(Reference ref, Ice.ObjectPrxHelperBase proxy)
    {
        if(ref.getCollocationOptimized())
        {
            Ice.ObjectAdapter adapter = _instance.objectAdapterFactory().findObjectAdapter(proxy);
            if(adapter != null)
            {
                return new CollocatedRequestHandler(ref, adapter);
            }
        }
        
        if(ref.getCacheConnection())
        {
            synchronized(this)
            {
                RequestHandler handler = _handlers.get(ref);
                if(handler != null)
                {
                    return handler;
                }
            
                handler = new ConnectRequestHandler(ref, proxy);
                if(_instance.queueRequests())
                {
                    handler = new QueueRequestHandler(_instance, handler);
                }
                _handlers.put(ref, handler);
                return handler;
            }
        }
        else
        {
            RequestHandler handler = new ConnectRequestHandler(ref, proxy);
            if(_instance.queueRequests())
            {
                handler = new QueueRequestHandler(_instance, handler);
            }
            return handler;
        }
    }

    void 
    removeRequestHandler(Reference ref, RequestHandler handler)
    {
        if(ref.getCacheConnection())
        {
            synchronized(this)
            {
                assert(_handlers.containsKey(ref));
                _handlers.remove(ref);
            }
        }
    }

    private final Instance _instance;
    private final Map<Reference, RequestHandler> _handlers = new HashMap<Reference, RequestHandler>();
}
