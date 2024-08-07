// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public sealed class RouterInfo : IEquatable<RouterInfo>
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
            _clientEndpoints = [];
            _adapter = null;
            _identities.Clear();
        }
    }

    public static bool operator ==(RouterInfo lhs, RouterInfo rhs) => lhs is not null ? lhs.Equals(rhs) : rhs is null;
    public static bool operator !=(RouterInfo lhs, RouterInfo rhs) => !(lhs == rhs);

    public bool Equals(RouterInfo other) =>
        ReferenceEquals(this, other) || (other is not null && _router.Equals(other._router));

    public override bool Equals(object obj) => Equals(obj as RouterInfo);

    public override int GetHashCode() => _router.GetHashCode();

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
        EndpointI[] clientEndpoints = null;
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
                    Debug.Assert(ae.InnerException is Ice.LocalException);
                    callback.setException((Ice.LocalException)ae.InnerException);
                }
            });
    }

    public EndpointI[] getServerEndpoints()
    {
        Ice.ObjectPrx serverProxy = _router.getServerProxy();
        if (serverProxy == null)
        {
            throw new NoEndpointException("Router::getServerProxy returned a null proxy.");
        }

        serverProxy = serverProxy.ice_router(null); // The server proxy cannot be routed.
        return ((Ice.ObjectPrxHelperBase)serverProxy).iceReference().getEndpoints();
    }

    public bool addProxy(Reference reference, AddProxyCallback callback)
    {
        Identity identity = reference.getIdentity();

        lock (this)
        {
            if (!_hasRoutingTable)
            {
                return true; // The router implementation doesn't maintain a routing table.
            }
            if (_identities.Contains(identity))
            {
                //
                // Only add the proxy to the router if it's not already in our local map.
                //
                return true;
            }
        }

        _router.addProxiesAsync([new ObjectPrxHelper(reference)]).ContinueWith(
            (t) =>
            {
                try
                {
                    addAndEvictProxies(identity, t.Result);
                    callback.addedProxy();
                }
                catch (AggregateException ae)
                {
                    Debug.Assert(ae.InnerException is Ice.LocalException);
                    callback.setException((Ice.LocalException)ae.InnerException);
                }
            });
        return false;
    }

    public void setAdapter(Ice.ObjectAdapter adapter)
    {
        lock (this)
        {
            _adapter = adapter;
        }
    }

    public Ice.ObjectAdapter getAdapter()
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

    private EndpointI[] setClientEndpoints(Ice.ObjectPrx clientProxy, bool hasRoutingTable)
    {
        lock (this)
        {
            if (_clientEndpoints is null)
            {
                _hasRoutingTable = hasRoutingTable;
                _clientEndpoints = clientProxy is null ?
                    ((ObjectPrxHelperBase)_router).iceReference().getEndpoints() :
                    ((ObjectPrxHelperBase)clientProxy).iceReference().getEndpoints();
            }
            return _clientEndpoints;
        }
    }

    private void addAndEvictProxies(Identity identity, Ice.ObjectPrx[] evictedProxies)
    {
        lock (this)
        {
            //
            // Check if the proxy hasn't already been evicted by a
            // concurrent addProxies call. If it's the case, don't
            // add it to our local map.
            //
            int index = _evictedIdentities.IndexOf(identity);
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
                _identities.Add(identity);
            }

            //
            // We also must remove whatever proxies the router evicted.
            //
            for (int i = 0; i < evictedProxies.Length; ++i)
            {
                if (!_identities.Remove(evictedProxies[i].ice_getIdentity()))
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
    public RouterInfo get(Ice.RouterPrx rtr)
    {
        if (rtr == null)
        {
            return null;
        }

        //
        // The router cannot be routed.
        //
        Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null));

        lock (this)
        {
            RouterInfo info = null;
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
    public RouterInfo erase(Ice.RouterPrx rtr)
    {
        RouterInfo info = null;
        if (rtr != null)
        {
            //
            // The router cannot be routed.
            //
            Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null));

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

    private Dictionary<Ice.RouterPrx, RouterInfo> _table;
}
