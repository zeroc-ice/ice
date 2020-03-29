//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using IceInternal;

namespace Ice
{
    public sealed partial class Communicator
    {
        internal IRequestHandler GetRequestHandler(RoutableReference rf, IObjectPrx proxy)
        {
            Debug.Assert(ReferenceEquals(rf, proxy.IceReference));
            if (rf.GetCollocationOptimized())
            {
                ObjectAdapter? adapter = FindObjectAdapter(proxy);
                if (adapter != null)
                {
                    return rf.SetRequestHandler(new CollocatedRequestHandler(rf, adapter));
                }
            }

            bool connect = false;
            ConnectRequestHandler handler;
            if (rf.IsRequestHandlerCached)
            {
                lock (_handlers)
                {
                    if (!_handlers.TryGetValue(rf, out handler))
                    {
                        handler = new ConnectRequestHandler(rf, proxy);
                        _handlers.Add(rf, handler);
                        connect = true;
                    }
                }
            }
            else
            {
                handler = new ConnectRequestHandler(rf, proxy);
                connect = true;
            }

            if (connect)
            {
                rf.GetConnection(handler);
            }
            return rf.SetRequestHandler(handler.Connect(rf));
        }

        internal void RemoveConnectRequestHandler(RoutableReference rf, ConnectRequestHandler handler)
        {
            if (rf.IsRequestHandlerCached)
            {
                lock (_handlers)
                {
                    if (_handlers.TryGetValue(rf, out ConnectRequestHandler h) && h == handler)
                    {
                        _handlers.Remove(rf);
                    }
                }
            }
        }

        private readonly Dictionary<RoutableReference, ConnectRequestHandler> _handlers =
            new Dictionary<RoutableReference, ConnectRequestHandler>();
    }
}
