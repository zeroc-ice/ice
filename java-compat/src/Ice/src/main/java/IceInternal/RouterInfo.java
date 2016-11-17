// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class RouterInfo
{
    interface GetClientEndpointsCallback
    {
        void setEndpoints(EndpointI[] endpoints);
        void setException(Ice.LocalException ex);
    }

    interface AddProxyCallback
    {
        void addedProxy();
        void setException(Ice.LocalException ex);
    }

    RouterInfo(Ice.RouterPrx router)
    {
        _router = router;

        assert(_router != null);
    }

    synchronized public void
    destroy()
    {
        _clientEndpoints = new EndpointI[0];
        _serverEndpoints = new EndpointI[0];
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

    public Ice.RouterPrx
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

        return setClientEndpoints(_router.getClientProxy());
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

        _router.begin_getClientProxy(new Ice.Callback_Router_getClientProxy()
            {
                @Override
                public void
                response(Ice.ObjectPrx clientProxy)
                {
                    callback.setEndpoints(setClientEndpoints(clientProxy));
                }

                @Override
                public void
                exception(Ice.LocalException ex)
                {
                    callback.setException(ex);
                }
            });
    }

    public EndpointI[]
    getServerEndpoints()
    {
        synchronized(this)
        {
            if(_serverEndpoints != null) // Lazy initialization.
            {
                return _serverEndpoints;
            }
        }

        return setServerEndpoints(_router.getServerProxy());
    }

    public boolean
    addProxy(final Ice.ObjectPrx proxy, final AddProxyCallback callback)
    {
        assert(proxy != null);
        synchronized(this)
        {
            if(_identities.contains(proxy.ice_getIdentity()))
            {
                //
                // Only add the proxy to the router if it's not already in our local map.
                //
                return true;
            }
        }

        _router.begin_addProxies(new Ice.ObjectPrx[] { proxy },
            new Ice.Callback_Router_addProxies()
            {
                @Override
                public void
                response(Ice.ObjectPrx[] evictedProxies)
                {
                    addAndEvictProxies(proxy, evictedProxies);
                    callback.addedProxy();
                }

                @Override
                public void
                exception(Ice.LocalException ex)
                {
                    callback.setException(ex);
                }
            });

        return false;
    }

    public synchronized void
    setAdapter(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public synchronized Ice.ObjectAdapter
    getAdapter()
    {
        return _adapter;
    }

    public synchronized void clearCache(Reference ref)
    {
        _identities.remove(ref.getIdentity());
    }

    private synchronized EndpointI[]
    setClientEndpoints(Ice.ObjectPrx clientProxy)
    {
        if(_clientEndpoints == null)
        {
            if(clientProxy == null)
            {
                //
                // If getClientProxy() return nil, use router endpoints.
                //
                _clientEndpoints = ((Ice.ObjectPrxHelperBase)_router)._getReference().getEndpoints();
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

                _clientEndpoints = ((Ice.ObjectPrxHelperBase)clientProxy)._getReference().getEndpoints();
            }
        }
        return _clientEndpoints;
    }

    private synchronized EndpointI[]
    setServerEndpoints(Ice.ObjectPrx serverProxy)
    {
        if(serverProxy == null)
        {
            throw new Ice.NoEndpointException();
        }

        serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
        _serverEndpoints = ((Ice.ObjectPrxHelperBase)serverProxy)._getReference().getEndpoints();
        return _serverEndpoints;
    }

    private synchronized void
    addAndEvictProxies(Ice.ObjectPrx proxy, Ice.ObjectPrx[] evictedProxies)
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
        for(Ice.ObjectPrx p : evictedProxies)
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

    private final Ice.RouterPrx _router;
    private EndpointI[] _clientEndpoints;
    private EndpointI[] _serverEndpoints;
    private Ice.ObjectAdapter _adapter;
    private java.util.Set<Ice.Identity> _identities = new java.util.HashSet<Ice.Identity>();
    private java.util.List<Ice.Identity> _evictedIdentities = new java.util.ArrayList<Ice.Identity>();
}
