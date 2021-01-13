// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Discovery
{
    /// <summary>Servant class that implements the Slice interface Ice::LocatorRegistry.</summary>
    internal class LocatorRegistry : IAsyncLocatorRegistry
    {
        private readonly IObjectPrx _dummyIce1Proxy;
        private readonly IObjectPrx _dummyIce2Proxy;

        private readonly Dictionary<string, IObjectPrx> _ice1Adapters = new();
        private readonly Dictionary<string, IReadOnlyList<EndpointData>> _ice2Adapters = new();

        private readonly object _mutex = new();

        private readonly Dictionary<(string AdapterId, Protocol Protocol), HashSet<string>> _replicaGroups = new();

        public ValueTask RegisterAdapterEndpointsAsync(
            string adapterId,
            string replicaGroupId,
            EndpointData[] endpoints,
            Current current,
            CancellationToken cancel)
        {
            if (endpoints.Length == 0)
            {
                throw new InvalidArgumentException("endpoints cannot be empty", nameof(endpoints));
            }

            RegisterAdapterEndpoints(adapterId, replicaGroupId, Protocol.Ice2, endpoints, _ice2Adapters);
            return default;
        }

        public ValueTask SetAdapterDirectProxyAsync(
            string adapterId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel) =>
            SetReplicatedAdapterDirectProxyAsync(adapterId, "", proxy, current, cancel);

        public ValueTask SetReplicatedAdapterDirectProxyAsync(
           string adapterId,
           string replicaGroupId,
           IObjectPrx? proxy,
           Current current,
           CancellationToken cancel)
        {
            if (proxy != null)
            {
                RegisterAdapterEndpoints(adapterId, replicaGroupId, Protocol.Ice1, proxy, _ice1Adapters);
            }
            else
            {
                UnregisterAdapterEndpoints(adapterId, replicaGroupId, Protocol.Ice1, _ice1Adapters);
            }
            return default;
        }

        public ValueTask SetServerProcessProxyAsync(
            string serverId,
            IProcessPrx process,
            Current current,
            CancellationToken cancel) => default; // Ignored

        public ValueTask UnregisterAdapterEndpointsAsync(
            string adapterId,
            string replicaGroupId,
            Current current,
            CancellationToken cancel)
        {
            UnregisterAdapterEndpoints(adapterId, replicaGroupId, Protocol.Ice2, _ice2Adapters);
            return default;
        }

        internal LocatorRegistry(Communicator communicator)
        {
            _dummyIce1Proxy = IObjectPrx.Parse("dummy", communicator);
            _dummyIce2Proxy = IObjectPrx.Parse("ice:dummy", communicator);
        }

        internal (IObjectPrx? Proxy, bool IsReplicaGroup) FindAdapter(string adapterId)
        {
            lock (_mutex)
            {
                if (_ice1Adapters.TryGetValue(adapterId, out IObjectPrx? proxy))
                {
                    return (proxy, false);
                }

                if (_replicaGroups.TryGetValue((adapterId, Protocol.Ice1), out HashSet<string>? adapterIds))
                {
                    Debug.Assert(adapterIds.Count > 0);
                    var endpoints = adapterIds.SelectMany(id => _ice1Adapters[id].Endpoints).ToList();
                    return (_dummyIce1Proxy.Clone(endpoints: endpoints), true);
                }

                return (null, false);
            }
        }

        internal async ValueTask<IObjectPrx?> FindObjectAsync(
            Identity identity,
            string? facet,
            CancellationToken cancel)
        {
            if (identity.Name.Length == 0)
            {
                return null;
            }

            List<string> candidates;

            lock (_mutex)
            {
                // We check the local replica groups before the local adapters.

                candidates =
                    _replicaGroups.Keys.Where(k => k.Protocol == Protocol.Ice1).Select(k => k.AdapterId).ToList();

                candidates.AddRange(_ice1Adapters.Keys);
            }

            foreach (string id in candidates)
            {
                try
                {
                    // This proxy is an indirect proxy with a location (the replica group ID or adapter ID).
                    IObjectPrx proxy = _dummyIce1Proxy.Clone(
                        IObjectPrx.Factory,
                        identity: identity,
                        facet: facet,
                        location: ImmutableArray.Create(id));
                    await proxy.IcePingAsync(cancel: cancel).ConfigureAwait(false);
                    return proxy;
                }
                catch
                {
                    // Ignore and move on to the next replica group ID / adapter ID
                }
            }

            return null;
        }

        internal (IReadOnlyList<EndpointData> Endpoints, bool IsReplicaGroup) ResolveAdapterId(string adapterId)
        {
            lock (_mutex)
            {
                if (_ice2Adapters.TryGetValue(adapterId, out IReadOnlyList<EndpointData>? endpoints))
                {
                    return (endpoints, false);
                }

                if (_replicaGroups.TryGetValue((adapterId, Protocol.Ice2), out HashSet<string>? adapterIds))
                {
                    Debug.Assert(adapterIds.Count > 0);
                    return (adapterIds.SelectMany(id => _ice2Adapters[id]).ToList(), true);
                }

                return (ImmutableArray<EndpointData>.Empty, false);
            }
        }

        internal async ValueTask<string> ResolveWellKnownProxyAsync(
            Identity identity,
            string facet,
            CancellationToken cancel)
        {
            if (identity.Name.Length == 0)
            {
                return "";
            }

            List<string> candidates;

            lock (_mutex)
            {
                // We check the local replica groups before the local adapters.

                candidates =
                    _replicaGroups.Keys.Where(k => k.Protocol == Protocol.Ice2).Select(k => k.AdapterId).ToList();

                candidates.AddRange(_ice2Adapters.Keys);
            }

            foreach (string id in candidates)
            {
                try
                {
                    // This proxy is an indirect proxy with a location (the replica group ID or adapter ID).
                    IObjectPrx proxy = _dummyIce2Proxy.Clone(
                        IObjectPrx.Factory,
                        identity: identity,
                        facet: facet,
                        location: ImmutableArray.Create(id));
                    await proxy.IcePingAsync(cancel: cancel).ConfigureAwait(false);
                    return id;
                }
                catch
                {
                    // Ignore and move on to the next replica group ID / adapter ID
                }
            }
            return "";
        }

        private void RegisterAdapterEndpoints<T>(
            string adapterId,
            string replicaGroupId,
            Protocol protocol,
            T endpoints,
            Dictionary<string, T> adapters)
        {
            if (adapterId.Length == 0)
            {
                throw new InvalidArgumentException("adapterId cannot be empty", nameof(adapterId));
            }

            lock (_mutex)
            {
                adapters[adapterId] = endpoints;
                if (replicaGroupId.Length > 0)
                {
                    if (!_replicaGroups.TryGetValue((replicaGroupId, protocol), out HashSet<string>? adapterIds))
                    {
                        adapterIds = new();
                        _replicaGroups.Add((replicaGroupId, protocol), adapterIds);
                    }
                    adapterIds.Add(adapterId);
                }
            }
        }

        private void UnregisterAdapterEndpoints<T>(
            string adapterId,
            string replicaGroupId,
            Protocol protocol,
            Dictionary<string, T> adapters)
        {
            if (adapterId.Length == 0)
            {
                throw new InvalidArgumentException("adapterId cannot be empty", nameof(adapterId));
            }

            lock (_mutex)
            {
                adapters.Remove(adapterId);
                if (replicaGroupId.Length > 0)
                {
                    if (_replicaGroups.TryGetValue((replicaGroupId, protocol), out HashSet<string>? adapterIds))
                    {
                        adapterIds.Remove(adapterId);
                        if (adapterIds.Count == 0)
                        {
                            _replicaGroups.Remove((replicaGroupId, protocol));
                        }
                    }
                }
            }
        }
    }
}
