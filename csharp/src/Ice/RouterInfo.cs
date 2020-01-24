//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using IceInternal;

namespace Ice
{
    public sealed class RouterInfo
    {
        public interface GetClientEndpointsCallback
        {
            void setEndpoints(Endpoint[] endpoints);
            void setException(Ice.LocalException ex);
        }

        public interface AddProxyCallback
        {
            void addedProxy();
            void SetException(Ice.LocalException ex);
        }

        internal RouterInfo(IRouterPrx router) => Router = router;

        public void Destroy()
        {
            lock (this)
            {
                _clientEndpoints = System.Array.Empty<Endpoint>();
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
            return rhs == null ? false : Router.Equals(rhs.Router);
        }

        public override int GetHashCode() => Router.GetHashCode();

        // No mutex lock necessary, _router is immutable.
        public IRouterPrx Router { get; }

        public Endpoint[] GetClientEndpoints()
        {
            lock (this)
            {
                if (_clientEndpoints != null) // Lazy initialization.
                {
                    return _clientEndpoints;
                }
            }

            var (proxy, hasRoutingTable) = Router.GetClientProxy();
            return SetClientEndpoints(proxy, hasRoutingTable.HasValue ? hasRoutingTable.Value : true);
        }

        public void GetClientEndpoints(GetClientEndpointsCallback callback)
        {
            Endpoint[]? clientEndpoints = null;
            lock (this)
            {
                clientEndpoints = _clientEndpoints;
            }

            if (clientEndpoints != null) // Lazy initialization.
            {
                callback.setEndpoints(clientEndpoints);
                return;
            }

            Router.GetClientProxyAsync().ContinueWith(
                (t) =>
                {
                    try
                    {
                        var r = t.Result;
                        callback.setEndpoints(SetClientEndpoints(r.ReturnValue,
                            r.HasRoutingTable.HasValue ? r.HasRoutingTable.Value : true));
                    }
                    catch (System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is LocalException);
                        callback.setException((LocalException)ae.InnerException);
                    }
                },
                System.Threading.Tasks.TaskScheduler.Current);
        }

        public Endpoint[] GetServerEndpoints()
        {
            IObjectPrx serverProxy = Router.GetServerProxy();
            if (serverProxy == null)
            {
                throw new NoEndpointException();
            }

            serverProxy = serverProxy.Clone(clearRouter: true); // The server proxy cannot be routed.
            return serverProxy.IceReference.GetEndpoints();
        }

        public void AddProxy(IObjectPrx proxy)
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

            AddAndEvictProxies(proxy, Router.AddProxies(new IObjectPrx[] { proxy }));
        }

        public bool AddProxy(IObjectPrx proxy, AddProxyCallback callback)
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

            Router.AddProxiesAsync(new IObjectPrx[] { proxy }).ContinueWith(
                (t) =>
                {
                    try
                    {
                        AddAndEvictProxies(proxy, t.Result);
                        callback.addedProxy();
                    }
                    catch (System.AggregateException ae)
                    {
                        Debug.Assert(ae.InnerException is LocalException);
                        callback.SetException((LocalException)ae.InnerException);
                    }
                },
                System.Threading.Tasks.TaskScheduler.Current);
            return false;
        }

        public ObjectAdapter? Adapter
        {
            get
            {
                lock (this)
                {
                    return _adapter;
                }
            }
            set
            {
                lock (this)
                {
                    _adapter = value;
                }
            }
        }

        public void ClearCache(Reference reference)
        {
            lock (this)
            {
                _identities.Remove(reference.GetIdentity());
            }
        }

        private Endpoint[] SetClientEndpoints(IObjectPrx clientProxy, bool hasRoutingTable)
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
                        _clientEndpoints = Router.IceReference.GetEndpoints();
                    }
                    else
                    {
                        clientProxy = clientProxy.Clone(clearRouter: true); // The client proxy cannot be routed.

                        //
                        // In order to avoid creating a new connection to the
                        // router, we must use the same timeout as the already
                        // existing connection.
                        //
                        if (Router.GetConnection() != null)
                        {
                            clientProxy = clientProxy.Clone(connectionTimeout: Router.GetConnection().Timeout);
                        }

                        _clientEndpoints = clientProxy.IceReference.GetEndpoints();
                    }
                }
                return _clientEndpoints;
            }
        }

        private void AddAndEvictProxies(IObjectPrx proxy, IObjectPrx[] evictedProxies)
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

        private Endpoint[]? _clientEndpoints;
        private ObjectAdapter? _adapter;
        private HashSet<Identity> _identities = new HashSet<Identity>();
        private List<Identity> _evictedIdentities = new List<Identity>();
        private bool _hasRoutingTable;
    }

    public sealed partial class Communicator
    {
        // Returns router info for a given router. Automatically creates
        // the router info if it doesn't exist yet.
        public RouterInfo GetRouterInfo(IRouterPrx rtr)
        {
            //
            // The router cannot be routed.
            //
            IRouterPrx router = rtr.Clone(clearRouter: true);

            lock (_routerInfoTable)
            {
                RouterInfo info;
                if (!_routerInfoTable.TryGetValue(router, out info))
                {
                    info = new RouterInfo(router);
                    _routerInfoTable.Add(router, info);
                }
                return info;
            }
        }

        //
        // Returns router info for a given router. Automatically creates
        // the router info if it doesn't exist yet.
        //
        public RouterInfo? EraseRouterInfo(IRouterPrx? rtr)
        {
            RouterInfo? info = null;
            if (rtr != null)
            {
                //
                // The router cannot be routed.
                //
                IRouterPrx router = rtr.Clone(clearRouter: true);

                lock (_routerInfoTable)
                {
                    if (_routerInfoTable.TryGetValue(router, out info))
                    {
                        _routerInfoTable.Remove(router);
                    }
                }
            }
            return info;
        }

        private Dictionary<IRouterPrx, RouterInfo> _routerInfoTable = new Dictionary<IRouterPrx, RouterInfo>();
    }

}
