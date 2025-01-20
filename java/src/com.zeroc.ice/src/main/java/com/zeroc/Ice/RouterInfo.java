// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class RouterInfo {
    interface GetClientEndpointsCallback {
        void setEndpoints(EndpointI[] endpoints);

        void setException(LocalException ex);
    }

    interface AddProxyCallback {
        void addedProxy();

        void setException(LocalException ex);
    }

    RouterInfo(RouterPrx router) {
        _router = router;

        assert (_router != null);
    }

    public synchronized void destroy() {
        _clientEndpoints = new EndpointI[0];
        _adapter = null;
        _identities.clear();
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (this == obj) {
            return true;
        }

        if (obj instanceof RouterInfo) {
            return _router.equals(((RouterInfo) obj)._router);
        }

        return false;
    }

    @Override
    public int hashCode() {
        return _router.hashCode();
    }

    public RouterPrx getRouter() {
        //
        // No mutex lock necessary, _router is immutable.
        //
        return _router;
    }

    public EndpointI[] getClientEndpoints() {
        synchronized (this) {
            if (_clientEndpoints != null) // Lazy initialization.
            {
                return _clientEndpoints;
            }
        }

        Router.GetClientProxyResult r = _router.getClientProxy();
        return setClientEndpoints(r.returnValue, r.hasRoutingTable.orElse(true));
    }

    public void getClientEndpoints(final GetClientEndpointsCallback callback) {
        EndpointI[] clientEndpoints = null;
        synchronized (this) {
            clientEndpoints = _clientEndpoints;
        }

        if (clientEndpoints != null) {
            callback.setEndpoints(clientEndpoints);
            return;
        }

        _router.getClientProxyAsync()
                .whenComplete(
                        (Router.GetClientProxyResult r, Throwable ex) -> {
                            if (ex != null) {
                                if (ex instanceof LocalException) {
                                    callback.setException((LocalException) ex);
                                } else {
                                    callback.setException(new UnknownException(ex));
                                }
                            } else {
                                callback.setEndpoints(
                                        setClientEndpoints(
                                                r.returnValue, r.hasRoutingTable.orElse(true)));
                            }
                        });
    }

    public EndpointI[] getServerEndpoints() {
        ObjectPrx serverProxy = _router.getServerProxy();
        if (serverProxy == null) {
            throw new NoEndpointException("Router::getServerProxy returned a null proxy.");
        }
        serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
        return ((_ObjectPrxI) serverProxy)._getReference().getEndpoints();
    }

    public boolean addProxy(final Reference reference, final AddProxyCallback callback) {
        Identity identity = reference.getIdentity();

        synchronized (this) {
            if (!_hasRoutingTable) {
                return true; // The router implementation doesn't maintain a routing table.
            }
            if (_identities.contains(identity)) {
                //
                // Only add the proxy to the router if it's not already in our local map.
                //
                return true;
            }
        }

        _router.addProxiesAsync(new ObjectPrx[] {new _ObjectPrxI(reference)})
                .whenComplete(
                        (ObjectPrx[] evictedProxies, Throwable ex) -> {
                            if (ex != null) {
                                if (ex instanceof LocalException) {
                                    callback.setException((LocalException) ex);
                                } else {
                                    callback.setException(new UnknownException(ex));
                                }
                            } else {
                                addAndEvictProxies(identity, evictedProxies);
                                callback.addedProxy();
                            }
                        });

        return false;
    }

    public synchronized void setAdapter(ObjectAdapter adapter) {
        _adapter = adapter;
    }

    public synchronized ObjectAdapter getAdapter() {
        return _adapter;
    }

    public synchronized void clearCache(Reference ref) {
        _identities.remove(ref.getIdentity());
    }

    private synchronized EndpointI[] setClientEndpoints(
            ObjectPrx clientProxy, boolean hasRoutingTable) {
        if (_clientEndpoints == null) {
            _hasRoutingTable = hasRoutingTable;
            _clientEndpoints =
                    clientProxy == null
                            ? ((_ObjectPrxI) _router)._getReference().getEndpoints()
                            : ((_ObjectPrxI) clientProxy)._getReference().getEndpoints();
        }
        return _clientEndpoints;
    }

    private synchronized void addAndEvictProxies(Identity identity, ObjectPrx[] evictedProxies) {
        //
        // Check if the proxy hasn't already been evicted by a
        // concurrent addProxies call. If it's the case, don't
        // add it to our local map.
        //
        int index = _evictedIdentities.indexOf(identity);
        if (index >= 0) {
            _evictedIdentities.remove(index);
        } else {
            //
            // If we successfully added the proxy to the router,
            // we add it to our local map.
            //
            _identities.add(identity);
        }

        //
        // We also must remove whatever proxies the router evicted.
        //
        for (ObjectPrx p : evictedProxies) {
            if (!_identities.remove(p.ice_getIdentity())) {
                //
                // It's possible for the proxy to not have been
                // added yet in the local map if two threads
                // concurrently call addProxies.
                //
                _evictedIdentities.add(p.ice_getIdentity());
            }
        }
    }

    private final RouterPrx _router;
    private EndpointI[] _clientEndpoints;
    private ObjectAdapter _adapter;
    private java.util.Set<Identity> _identities = new java.util.HashSet<>();
    private java.util.List<Identity> _evictedIdentities = new java.util.ArrayList<>();
    private boolean _hasRoutingTable;
}
