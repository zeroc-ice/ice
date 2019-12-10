//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public final class RouterInfo
{
    interface GetClientEndpointsCallback
    {
        void setEndpoints(EndpointI[] endpoints);
        void setException(com.zeroc.Ice.LocalException ex);
    }

    interface AddProxyCallback
    {
        void addedProxy();
        void setException(com.zeroc.Ice.LocalException ex);
    }

    RouterInfo(com.zeroc.Ice.RouterPrx router)
    {
        _router = router;

        assert(_router != null);
    }

    synchronized public void
    destroy()
    {
        _clientEndpoints = new EndpointI[0];
        _adapter = null;
        _identities.clear();
    }

    @Override
    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }

        if(obj instanceof RouterInfo)
        {
            return _router.equals(((RouterInfo)obj)._router);
        }

        return false;
    }

    @Override
    public int
    hashCode()
    {
        return _router.hashCode();
    }

    public com.zeroc.Ice.RouterPrx
    getRouter()
    {
        //
        // No mutex lock necessary, _router is immutable.
        //
        return _router;
    }

    public EndpointI[]
    getClientEndpoints()
    {
        synchronized(this)
        {
            if(_clientEndpoints != null) // Lazy initialization.
            {
                return _clientEndpoints;
            }
        }

        com.zeroc.Ice.Router.GetClientProxyResult r = _router.getClientProxy();
        return setClientEndpoints(r.returnValue, (r.hasRoutingTable != null ? r.hasRoutingTable : true));
    }

    public void
    getClientEndpoints(final GetClientEndpointsCallback callback)
    {
        EndpointI[] clientEndpoints = null;
        synchronized(this)
        {
            clientEndpoints = _clientEndpoints;
        }

        if(clientEndpoints != null)
        {
            callback.setEndpoints(clientEndpoints);
            return;
        }

        _router.getClientProxyAsync().whenComplete((com.zeroc.Ice.Router.GetClientProxyResult r, Throwable ex) ->
            {
                if(ex != null)
                {
                    if(ex instanceof com.zeroc.Ice.LocalException)
                    {
                        callback.setException((com.zeroc.Ice.LocalException)ex);
                    }
                    else
                    {
                        callback.setException(new com.zeroc.Ice.UnknownException(ex));
                    }
                }
                else
                {
                    callback.setEndpoints(setClientEndpoints(r.returnValue,
                                                             (r.hasRoutingTable != null? r.hasRoutingTable : true)));
                }
            });
    }

    public EndpointI[]
    getServerEndpoints()
    {
        com.zeroc.Ice.ObjectPrx serverProxy = _router.getServerProxy();
        if(serverProxy == null)
        {
            throw new com.zeroc.Ice.NoEndpointException();
        }
        serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
        return ((com.zeroc.Ice._ObjectPrxI)serverProxy)._getReference().getEndpoints();
    }

    public boolean
    addProxy(final com.zeroc.Ice.ObjectPrx proxy, final AddProxyCallback callback)
    {
        assert(proxy != null);
        synchronized(this)
        {
            if(!_hasRoutingTable)
            {
                return true; // The router implementation doesn't maintain a routing table.
            }
            if(_identities.contains(proxy.ice_getIdentity()))
            {
                //
                // Only add the proxy to the router if it's not already in our local map.
                //
                return true;
            }
        }

        _router.addProxiesAsync(new com.zeroc.Ice.ObjectPrx[] { proxy }).whenComplete(
            (com.zeroc.Ice.ObjectPrx[] evictedProxies, Throwable ex) ->
            {
                if(ex != null)
                {
                    if(ex instanceof com.zeroc.Ice.LocalException)
                    {
                        callback.setException((com.zeroc.Ice.LocalException)ex);
                    }
                    else
                    {
                        callback.setException(new com.zeroc.Ice.UnknownException(ex));
                    }
                }
                else
                {
                    addAndEvictProxies(proxy, evictedProxies);
                    callback.addedProxy();
                }
            });

        return false;
    }

    public synchronized void
    setAdapter(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public synchronized com.zeroc.Ice.ObjectAdapter
    getAdapter()
    {
        return _adapter;
    }

    public synchronized void clearCache(Reference ref)
    {
        _identities.remove(ref.getIdentity());
    }

    private synchronized EndpointI[]
    setClientEndpoints(com.zeroc.Ice.ObjectPrx clientProxy, boolean hasRoutingTable)
    {
        if(_clientEndpoints == null)
        {
            _hasRoutingTable = hasRoutingTable;
            if(clientProxy == null)
            {
                //
                // If getClientProxy() return nil, use router endpoints.
                //
                _clientEndpoints = ((com.zeroc.Ice._ObjectPrxI)_router)._getReference().getEndpoints();
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

                _clientEndpoints = ((com.zeroc.Ice._ObjectPrxI)clientProxy)._getReference().getEndpoints();
            }
        }
        return _clientEndpoints;
    }

    private synchronized void
    addAndEvictProxies(com.zeroc.Ice.ObjectPrx proxy, com.zeroc.Ice.ObjectPrx[] evictedProxies)
    {
        //
        // Check if the proxy hasn't already been evicted by a
        // concurrent addProxies call. If it's the case, don't
        // add it to our local map.
        //
        int index = _evictedIdentities.indexOf(proxy.ice_getIdentity());
        if(index >= 0)
        {
            _evictedIdentities.remove(index);
        }
        else
        {
            //
            // If we successfully added the proxy to the router,
            // we add it to our local map.
            //
            _identities.add(proxy.ice_getIdentity());
        }

        //
        // We also must remove whatever proxies the router evicted.
        //
        for(com.zeroc.Ice.ObjectPrx p : evictedProxies)
        {
            if(!_identities.remove(p.ice_getIdentity()))
            {
                //
                // It's possible for the proxy to not have been
                // added yet in the local map if two threads
                // concurrently call addProxies.
                //
                _evictedIdentities.add(p.ice_getIdentity());
            }
        }
    }

    private final com.zeroc.Ice.RouterPrx _router;
    private EndpointI[] _clientEndpoints;
    private com.zeroc.Ice.ObjectAdapter _adapter;
    private java.util.Set<com.zeroc.Ice.Identity> _identities = new java.util.HashSet<>();
    private java.util.List<com.zeroc.Ice.Identity> _evictedIdentities = new java.util.ArrayList<>();
    private boolean _hasRoutingTable;
}
