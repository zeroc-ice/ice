// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
    public class RequestHandlerFactory
    {
        internal RequestHandlerFactory(Instance instance)
        {
            _instance = instance;
        }

        public RequestHandler 
        getRequestHandler(Reference rf, Ice.ObjectPrxHelperBase proxy)
        {
            if(rf.getCollocationOptimized())
            {
                Ice.ObjectAdapter adapter = _instance.objectAdapterFactory().findObjectAdapter(proxy);
                if(adapter != null)
                {
                    return new CollocatedRequestHandler(rf, adapter);
                }
            }
            
            if(rf.getCacheConnection())
            {
                lock(this)
                {
                    RequestHandler handler;
                    if(_handlers.TryGetValue(rf, out handler))
                    {
                        return handler;
                    }
                    
                    handler = new ConnectRequestHandler(rf, proxy);
                    _handlers.Add(rf, handler);
                    return handler;
                }
            }
            else
            {
                return new ConnectRequestHandler(rf, proxy);
            }
        }
        
        internal void 
        removeRequestHandler(Reference rf, RequestHandler handler)
        {
            if(rf.getCacheConnection())
            {
                lock(this)
                {
                    RequestHandler h;
                    if(_handlers.TryGetValue(rf, out h) && h == handler)
                    {
                        _handlers.Remove(rf);
                    }
                }
            }
        }
        
        readonly Instance _instance;
        readonly Dictionary<Reference, RequestHandler> _handlers = new Dictionary<Reference, RequestHandler>();
    }
}
