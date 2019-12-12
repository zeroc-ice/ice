//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;
    using Ice;

    public sealed class RouterInfo
    {
        public interface GetClientEndpointsCallback
        {
            void setEndpoints(EndpointI[] endpoints);
            void setException(Ice.LocalException ex);
        }

        public interface AddProxyCallback
        {
            void addedProxy();
            void setException(Ice.LocalException ex);
        }

        internal RouterInfo(Ice.RouterPrx router)
        {
            _router = router;

            Debug.Assert(_router != null);
        }

        public void destroy()
        {
            lock (this)
            {
                _clientEndpoints = System.Array.Empty<EndpointI>();
                _adapter = null;
                _identities.Clear();
            }
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            RouterInfo? rhs = obj as RouterInfo;
            return rhs == null ? false : _router.Equals(rhs._router);
        }

        public override int GetHashCode()
        {
            return _router.GetHashCode();
        }

        public Ice.RouterPrx getRouter()
        {
            //
            // No mutex lock necessary, _router is immutable.
            //
            return _router;
        }

        public EndpointI[] getClientEndpoints()
        {
            lock (this)
            {
                if (_clientEndpoints != null) // Lazy initialization.
                {
                    return _clientEndpoints;
                }
            }

            bool? hasRoutingTable;
            var proxy = _router.getClientProxy(out hasRoutingTable);
            return setClientEndpoints(proxy, hasRoutingTable.HasValue ? hasRoutingTable.Value : true);
        }

        public void getClientEndpoints(GetClientEndpointsCallback callback)
        {
            EndpointI[]? clientEndpoints = null;
            lock (this)
            {
                clientEndpoints = _clientEndpoints;
            }

            if (clientEndpoints != null) // Lazy initialization.
            {
                callback.setEndpoints(clientEndpoints);
                return;
            }

            _router.getClientProxyAsync().ContinueWith(
                (t) =>
                {
                    try
                    {
                        var r = t.Result;
                        callback.setEndpoints(setClientEndpoints(r.returnValue,
                                                    r.hasRoutingTable.HasValue ? r.hasRoutingTable.Value : true));
                    }
                    catch (System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is LocalException);
                        callback.setException((LocalException)ae.InnerException);
                    }
                },
                System.Threading.Tasks.TaskScheduler.Current);
        }

        public EndpointI[] getServerEndpoints()
        {
            Ice.IObjectPrx serverProxy = _router.getServerProxy();
            if (serverProxy == null)
            {
                throw new Ice.NoEndpointException();
            }

            serverProxy = serverProxy.Clone(clearRouter: true); // The server proxy cannot be routed.
            return serverProxy.IceReference.getEndpoints();
        }

        public void addProxy(Ice.IObjectPrx proxy)
        {
            Debug.Assert(proxy != null);
            lock (this)
            {
                if (_identities.Contains(proxy.Identity))
                {
                    //
                    // Only add the proxy to the router if it's not already in our local map.
                    //
                    return;
                }
            }

            addAndEvictProxies(proxy, _router.addProxies(new Ice.IObjectPrx[] { proxy }));
        }

        public bool addProxy(Ice.IObjectPrx proxy, AddProxyCallback callback)
        {
            Debug.Assert(proxy != null);
            lock (this)
            {
                if (!_hasRoutingTable)
                {
                    return true; // The router implementation doesn't maintain a routing table.
                }
                if (_identities.Contains(proxy.Identity))
                {
                    //
                    // Only add the proxy to the router if it's not already in our local map.
                    //
                    return true;
                }
            }

            _router.addProxiesAsync(new Ice.IObjectPrx[] { proxy }).ContinueWith(
                (t) =>
                {
                    try
                    {
                        addAndEvictProxies(proxy, t.Result);
                        callback.addedProxy();
                    }
                    catch (System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is Ice.LocalException);
                        callback.setException((Ice.LocalException)ae.InnerException);
                    }
                },
                System.Threading.Tasks.TaskScheduler.Current);
            return false;
        }

        public void setAdapter(Ice.ObjectAdapter? adapter)
        {
            lock (this)
            {
                _adapter = adapter;
            }
        }

        public Ice.ObjectAdapter? getAdapter()
        {
            lock (this)
            {
                return _adapter;
            }
        }

        public void clearCache(Reference @ref)
        {
            lock (this)
            {
                _identities.Remove(@ref.getIdentity());
            }
        }

        private EndpointI[] setClientEndpoints(Ice.IObjectPrx clientProxy, bool hasRoutingTable)
        {
            lock (this)
            {
                if (_clientEndpoints == null)
                {
                    _hasRoutingTable = hasRoutingTable;
                    if (clientProxy == null)
                    {
                        //
                        // If getClientProxy() return nil, use router endpoints.
                        //
                        _clientEndpoints = _router.IceReference.getEndpoints();
                    }
                    else
                    {
                        clientProxy = clientProxy.Clone(clearRouter: true); // The client proxy cannot be routed.

                        //
                        // In order to avoid creating a new connection to the
                        // router, we must use the same timeout as the already
                        // existing connection.
                        //
                        if (_router.GetConnection() != null)
                        {
                            clientProxy = clientProxy.Clone(connectionTimeout: _router.GetConnection().timeout());
                        }

                        _clientEndpoints = clientProxy.IceReference.getEndpoints();
                    }
                }
                return _clientEndpoints;
            }
        }

        private void addAndEvictProxies(Ice.IObjectPrx proxy, Ice.IObjectPrx[] evictedProxies)
        {
            lock (this)
            {
                //
                // Check if the proxy hasn't already been evicted by a
                // concurrent addProxies call. If it's the case, don't
                // add it to our local map.
                //
                int index = _evictedIdentities.IndexOf(proxy.Identity);
                if (index >= 0)
                {
                    _evictedIdentities.RemoveAt(index);
                }
                else
                {
                    //
                    // If we successfully added the proxy to the router,
                    // we add it to our local map.
                    //
                    _identities.Add(proxy.Identity);
                }

                //
                // We also must remove whatever proxies the router evicted.
                //
                for (int i = 0; i < evictedProxies.Length; ++i)
                {
                    if (!_identities.Remove(evictedProxies[i].Identity))
                    {
                        //
                        // It's possible for the proxy to not have been
                        // added yet in the local map if two threads
                        // concurrently call addProxies.
                        //
                        _evictedIdentities.Add(evictedProxies[i].Identity);
                    }
                }
            }
        }

        private readonly RouterPrx _router;
        private EndpointI[]? _clientEndpoints;
        private ObjectAdapter? _adapter;
        private HashSet<Identity> _identities = new HashSet<Identity>();
        private List<Identity> _evictedIdentities = new List<Identity>();
        private bool _hasRoutingTable;
    }

    public sealed class RouterManager
    {
        internal RouterManager()
        {
            _table = new Dictionary<RouterPrx, RouterInfo>();
        }

        internal void destroy()
        {
            lock (this)
            {
                foreach (RouterInfo i in _table.Values)
                {
                    i.destroy();
                }
                _table.Clear();
            }
        }

        //
        // Returns router info for a given router. Automatically creates
        // the router info if it doesn't exist yet.
        //
        public RouterInfo get(RouterPrx rtr)
        {
            //
            // The router cannot be routed.
            //
            RouterPrx router = rtr.Clone(clearRouter: true);

            lock (this)
            {
                RouterInfo info;
                if (!_table.TryGetValue(router, out info))
                {
                    info = new RouterInfo(router);
                    _table.Add(router, info);
                }
                return info;
            }
        }

        //
        // Returns router info for a given router. Automatically creates
        // the router info if it doesn't exist yet.
        //
        public RouterInfo? erase(RouterPrx? rtr)
        {
            RouterInfo? info = null;
            if (rtr != null)
            {
                //
                // The router cannot be routed.
                //
                RouterPrx router = rtr.Clone(clearRouter: true);

                lock (this)
                {
                    if (_table.TryGetValue(router, out info))
                    {
                        _table.Remove(router);
                    }
                }
            }
            return info;
        }

        private Dictionary<RouterPrx, RouterInfo> _table;
    }

}
