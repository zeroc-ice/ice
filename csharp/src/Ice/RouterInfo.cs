// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;
    using System.Diagnostics;

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
            lock(this)
            {
                _clientEndpoints = new EndpointI[0];
                _adapter = null;
                _identities.Clear();
            }
        }

        public override bool Equals(object obj)
        {
            if(ReferenceEquals(this, obj))
            {
                return true;
            }

            RouterInfo rhs = obj as RouterInfo;
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
            lock(this)
            {
                if(_clientEndpoints != null) // Lazy initialization.
                {
                    return _clientEndpoints;
                }
            }

            Ice.Optional<bool> hasRoutingTable;
            var proxy = _router.getClientProxy(out hasRoutingTable);
            return setClientEndpoints(proxy, hasRoutingTable.HasValue ? hasRoutingTable.Value : true);
        }

        public void getClientEndpoints(GetClientEndpointsCallback callback)
        {
            EndpointI[] clientEndpoints = null;
            lock(this)
            {
                clientEndpoints = _clientEndpoints;
            }

            if(clientEndpoints != null) // Lazy initialization.
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
                    catch(System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is Ice.LocalException);
                        callback.setException((Ice.LocalException)ae.InnerException);
                    }
                });
        }

        public EndpointI[] getServerEndpoints()
        {
            Ice.ObjectPrx serverProxy = _router.getServerProxy();
            if(serverProxy == null)
            {
                throw new Ice.NoEndpointException();
            }

            serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
            return ((Ice.ObjectPrxHelperBase)serverProxy).iceReference().getEndpoints();
        }

        public void addProxy(Ice.ObjectPrx proxy)
        {
            Debug.Assert(proxy != null);
            lock(this)
            {
                if(_identities.Contains(proxy.ice_getIdentity()))
                {
                    //
                    // Only add the proxy to the router if it's not already in our local map.
                    //
                    return;
                }
            }

            addAndEvictProxies(proxy, _router.addProxies(new Ice.ObjectPrx[] { proxy }));
        }

        public bool addProxy(Ice.ObjectPrx proxy, AddProxyCallback callback)
        {
            Debug.Assert(proxy != null);
            lock(this)
            {
                if(!_hasRoutingTable)
                {
                    return true; // The router implementation doesn't maintain a routing table.
                }
                if(_identities.Contains(proxy.ice_getIdentity()))
                {
                    //
                    // Only add the proxy to the router if it's not already in our local map.
                    //
                    return true;
                }
            }

            _router.addProxiesAsync(new Ice.ObjectPrx[] { proxy }).ContinueWith(
                (t) =>
                {
                    try
                    {
                        addAndEvictProxies(proxy, t.Result);
                        callback.addedProxy();
                    }
                    catch(System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is Ice.LocalException);
                        callback.setException((Ice.LocalException)ae.InnerException);
                    }
                });
            return false;
        }

        public void setAdapter(Ice.ObjectAdapter adapter)
        {
            lock(this)
            {
                _adapter = adapter;
            }
        }

        public Ice.ObjectAdapter getAdapter()
        {
            lock(this)
            {
                return _adapter;
            }
        }

        public void clearCache(Reference @ref)
        {
            lock(this)
            {
                _identities.Remove(@ref.getIdentity());
            }
        }

        private EndpointI[] setClientEndpoints(Ice.ObjectPrx clientProxy, bool hasRoutingTable)
        {
            lock(this)
            {
                if(_clientEndpoints == null)
                {
                    _hasRoutingTable = hasRoutingTable;
                    if(clientProxy == null)
                    {
                        //
                        // If getClientProxy() return nil, use router endpoints.
                        //
                        _clientEndpoints = ((Ice.ObjectPrxHelperBase)_router).iceReference().getEndpoints();
                    }
                    else
                    {
                        clientProxy = clientProxy.ice_router(null); // The client proxy cannot be routed.

                        //
                        // In order to avoid creating a new connection to the
                        // router, we must use the same timeout as the already
                        // existing connection.
                        //
                        if(_router.ice_getConnection() != null)
                        {
                            clientProxy = clientProxy.ice_timeout(_router.ice_getConnection().timeout());
                        }

                        _clientEndpoints = ((Ice.ObjectPrxHelperBase)clientProxy).iceReference().getEndpoints();
                    }
                }
                return _clientEndpoints;
            }
        }

        private void addAndEvictProxies(Ice.ObjectPrx proxy, Ice.ObjectPrx[] evictedProxies)
        {
            lock(this)
            {
                //
                // Check if the proxy hasn't already been evicted by a
                // concurrent addProxies call. If it's the case, don't
                // add it to our local map.
                //
                int index = _evictedIdentities.IndexOf(proxy.ice_getIdentity());
                if(index >= 0)
                {
                    _evictedIdentities.RemoveAt(index);
                }
                else
                {
                    //
                    // If we successfully added the proxy to the router,
                    // we add it to our local map.
                    //
                    _identities.Add(proxy.ice_getIdentity());
                }

                //
                // We also must remove whatever proxies the router evicted.
                //
                for(int i = 0; i < evictedProxies.Length; ++i)
                {
                    if(!_identities.Remove(evictedProxies[i].ice_getIdentity()))
                    {
                        //
                        // It's possible for the proxy to not have been
                        // added yet in the local map if two threads
                        // concurrently call addProxies.
                        //
                        _evictedIdentities.Add(evictedProxies[i].ice_getIdentity());
                    }
                }
            }
        }

        private readonly Ice.RouterPrx _router;
        private EndpointI[] _clientEndpoints;
        private Ice.ObjectAdapter _adapter;
        private HashSet<Ice.Identity> _identities = new HashSet<Ice.Identity>();
        private List<Ice.Identity> _evictedIdentities = new List<Ice.Identity>();
        private bool _hasRoutingTable;
    }

    public sealed class RouterManager
    {
        internal RouterManager()
        {
            _table = new Dictionary<Ice.RouterPrx, RouterInfo>();
        }

        internal void destroy()
        {
            lock(this)
            {
                foreach(RouterInfo i in _table.Values)
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
        public RouterInfo get(Ice.RouterPrx rtr)
        {
            if(rtr == null)
            {
                return null;
            }

            //
            // The router cannot be routed.
            //
            Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null));

            lock(this)
            {
                RouterInfo info = null;
                if(!_table.TryGetValue(router, out info))
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
        public RouterInfo erase(Ice.RouterPrx rtr)
        {
            RouterInfo info = null;
            if(rtr != null)
            {
                //
                // The router cannot be routed.
                //
                Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null));

                lock(this)
                {
                    if(_table.TryGetValue(router, out info))
                    {
                        _table.Remove(router);
                    }
                }
            }
            return info;
        }

        private Dictionary<Ice.RouterPrx, RouterInfo> _table;
    }

}
