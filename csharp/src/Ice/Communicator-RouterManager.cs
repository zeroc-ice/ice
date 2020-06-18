//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal sealed class RouterInfo
    {
        // No mutex lock necessary, _router is immutable.
        public IRouterPrx Router { get; }
        public ObjectAdapter? Adapter
        {
            get => _adapter;
            set => _adapter = value;
        }
        private volatile ObjectAdapter? _adapter;
        private IReadOnlyList<Endpoint>? _clientEndpoints;
        private readonly List<Identity> _evictedIdentities = new List<Identity>();
        private bool _hasRoutingTable;
        private readonly HashSet<Identity> _identities = new HashSet<Identity>();
        private readonly object _mutex = new object();

        public override bool Equals(object? obj) =>
            ReferenceEquals(this, obj) || (obj is RouterInfo rhs && Router.Equals(rhs.Router));

        public override int GetHashCode() => Router.GetHashCode();

        internal RouterInfo(IRouterPrx router) => Router = router;

        internal async ValueTask AddProxyAsync(IObjectPrx proxy)
        {
            Debug.Assert(proxy != null);
            lock (_mutex)
            {
                if (!_hasRoutingTable)
                {
                    // The router implementation doesn't maintain a routing table.
                    return;
                }
                if (_identities.Contains(proxy.Identity))
                {
                    // Only add the proxy to the router if it's not already in our local map.
                    return;
                }
            }

            // TODO: fix the Slice method addProxies to return non-nullable proxies.
            IObjectPrx?[] evictedProxies = await Router.AddProxiesAsync(new IObjectPrx[] { proxy });

            lock (_mutex)
            {
                // Check if the proxy hasn't already been evicted by a concurrent addProxies call. If it's the case,
                // don't add it to our local map.
                int index = _evictedIdentities.IndexOf(proxy.Identity);
                if (index >= 0)
                {
                    _evictedIdentities.RemoveAt(index);
                }
                else
                {
                    // If we successfully added the proxy to the router, we add it to our local map.
                    _identities.Add(proxy.Identity);
                }

                // We also must remove whatever proxies the router evicted.
                for (int i = 0; i < evictedProxies.Length; ++i)
                {
                    if (evictedProxies[i] != null)
                    {
                        if (!_identities.Remove(evictedProxies[i]!.Identity))
                        {
                            // It's possible for the proxy to not have been added yet in the local map if two threads
                            // concurrently call addProxies.
                            _evictedIdentities.Add(evictedProxies[i]!.Identity);
                        }
                    }
                }
            }
        }

        internal void ClearCache(Reference reference)
        {
            lock (_mutex)
            {
                _identities.Remove(reference.Identity);
            }
        }

        internal async ValueTask<IReadOnlyList<Endpoint>> GetClientEndpointsAsync()
        {
            lock (_mutex)
            {
                if (_clientEndpoints != null) // Lazy initialization.
                {
                    return _clientEndpoints;
                }
            }

            (IObjectPrx? clientProxy, bool? hasRoutingTable) = await Router.GetClientProxyAsync().ConfigureAwait(false);

            lock (_mutex)
            {
                if (_clientEndpoints == null)
                {
                    _hasRoutingTable = hasRoutingTable ?? true;
                    if (clientProxy == null)
                    {
                        //
                        // If getClientProxy() return nil, use router endpoints.
                        //
                        _clientEndpoints = Router.IceReference.Endpoints;
                    }
                    else
                    {
                        clientProxy = clientProxy.Clone(clearRouter: true); // The client proxy cannot be routed.

                        //
                        // In order to avoid creating a new connection to the
                        // router, we must use the same timeout as the already
                        // existing connection.
                        //
                        Connection? connection = Router.GetConnection();
                        if (connection != null)
                        {
                            clientProxy = clientProxy.Clone(connectionTimeout: connection.Timeout);
                        }

                        _clientEndpoints = clientProxy.IceReference.Endpoints;
                    }
                }
                return _clientEndpoints;
            }
        }

        internal IReadOnlyList<Endpoint> GetServerEndpoints()
        {
            IObjectPrx? serverProxy = Router.GetServerProxy();
            if (serverProxy == null)
            {
                throw new InvalidConfigurationException($"router `{Router.Identity}' has no server endpoints");
            }

            // The server proxy cannot be routed.
            return serverProxy.Clone(clearRouter: true).IceReference.Endpoints;
        }
    }

    public sealed partial class Communicator
    {
        private readonly Dictionary<IRouterPrx, RouterInfo> _routerInfoTable = new Dictionary<IRouterPrx, RouterInfo>();

        // Returns router info for a given router. Automatically creates the router info if it doesn't exist yet.
        internal RouterInfo? GetRouterInfo(IRouterPrx? rtr)
        {
            if (rtr == null)
            {
                return null;
            }

            lock (_routerInfoTable)
            {
                // The router cannot be routed.
                IRouterPrx router = rtr.Clone(clearRouter: true);
                if (!_routerInfoTable.TryGetValue(router, out RouterInfo? info))
                {
                    info = new RouterInfo(router);
                    _routerInfoTable.Add(router, info);
                }
                return info;
            }
        }

        // Removes router info for a given router.
        internal void EraseRouterInfo(IRouterPrx? rtr)
        {
            if (rtr != null)
            {
                lock (_routerInfoTable)
                {
                    // The router cannot be routed.
                    _routerInfoTable.Remove(rtr.Clone(clearRouter: true));
                }
            }
        }
    }
}
