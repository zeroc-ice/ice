//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using IceInternal;

namespace Ice
{
    public sealed partial class Communicator
    {
        // All the reference here are routable references.

        internal IRequestHandler GetRequestHandler(Reference rf)
        {
            if (rf.IsCollocationOptimized)
            {
                ObjectAdapter? adapter = FindObjectAdapter(rf);
                if (adapter != null)
                {
                    return rf.SetRequestHandler(new CollocatedRequestHandler(rf, adapter));
                }
            }

            bool connect = false;
            ConnectRequestHandler? handler;
            if (rf.IsConnectionCached)
            {
                lock (_handlers)
                {
                    if (!_handlers.TryGetValue(rf, out handler))
                    {
                        handler = new ConnectRequestHandler(rf);
                        _handlers.Add(rf, handler);
                        connect = true;
                    }
                }
            }
            else
            {
                handler = new ConnectRequestHandler(rf);
                connect = true;
            }

            if (connect)
            {
                rf.GetConnection(handler);
            }
            return rf.SetRequestHandler(handler.Connect(rf));
        }

        internal void RemoveConnectRequestHandler(Reference rf, ConnectRequestHandler handler)
        {
            if (rf.IsConnectionCached)
            {
                lock (_handlers)
                {
                    if (_handlers.TryGetValue(rf, out ConnectRequestHandler? h) && h == handler)
                    {
                        _handlers.Remove(rf);
                    }
                }
            }
        }

        private readonly Dictionary<Reference, ConnectRequestHandler> _handlers =
            new Dictionary<Reference, ConnectRequestHandler>();
    }
}
