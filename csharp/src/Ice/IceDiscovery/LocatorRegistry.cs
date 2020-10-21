// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class LocatorRegistry : ILocatorRegistry
    {
        private readonly IObjectPrx _dummyIce1Proxy;
        private readonly IObjectPrx _dummyIce2Proxy;

        private readonly Dictionary<string, IObjectPrx> _ice1Adapters = new ();

        // Dictionary for protocol >= ice2.
        private readonly Dictionary<string, IReadOnlyList<EndpointData>> _ice2Adapters = new ();

        private readonly object _mutex = new ();

        private readonly Dictionary<(string, Protocol), HashSet<string>> _replicaGroups = new ();

        public void RegisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            EndpointData[] endpoints,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                try
                {
                    _ice2Adapters.Add(adapterId, endpoints);
                }
                catch (ArgumentException)
                {
                    throw new AdapterAlreadyActiveException($"adapter `{adapterId}' is already active");
                }

                AddAdapterToReplicaGroup(adapterId, replicaGroupId, Protocol.Ice2);
            }
        }

        public void SetAdapterDirectProxy(
            string adapterId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel) =>
            SetReplicatedAdapterDirectProxy(adapterId, "", proxy, current, cancel);

        public void SetReplicatedAdapterDirectProxy(
           string adapterId,
           string replicaGroupId,
           IObjectPrx? proxy,
           Current current,
           CancellationToken cancel)
        {
            lock (_mutex)
            {
                if (proxy != null)
                {
                    try
                    {
                        _ice1Adapters.Add(adapterId, proxy);
                    }
                    catch (ArgumentException)
                    {
                        throw new AdapterAlreadyActiveException($"adapter `{adapterId}' is already active");
                    }
                    AddAdapterToReplicaGroup(adapterId, replicaGroupId, Protocol.Ice1);
                }
                else
                {
                    _ice1Adapters.Remove(adapterId);
                    RemoveAdapterFromReplicaGroup(adapterId, replicaGroupId, Protocol.Ice1);
                }
            }
        }

        public void SetServerProcessProxy(
            string serverId,
            IProcessPrx process,
            Current current,
            CancellationToken cancel)
        {
            // Ignored
        }

        public void UnregisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                _ice2Adapters.Remove(adapterId);
                RemoveAdapterFromReplicaGroup(adapterId, replicaGroupId, Protocol.Ice2);
            }
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

                    IObjectPrx? result = null;
                    var endpoints = new List<Endpoint>();
                    foreach (string id in adapterIds)
                    {
                        // We assume that if adapterId is in adapterIds, it's also in the _ice1Adapters dictionary.
                        // The two dictionaries can be out of sync if there is a bug in the local Ice runtime or
                        // application code that uses this local colocated LocatorRegistry directly. For example, a call
                        // to unregisterAdapterEndpoints with a missing or incorrect replicaGroupId. If there is such a
                        // local bug, the code will throw a KeyNotFoundException.
                        IObjectPrx p = _ice1Adapters[id];
                        result ??= p;
                        endpoints.AddRange(p.Endpoints);
                    }

                    return (result?.Clone(endpoints: endpoints), result != null);
                }

                return (null, false);
            }
        }

        internal IObjectPrx? FindObject(Identity identity)
        {
            lock (_mutex)
            {
                if (identity.Name.Length == 0)
                {
                    return null;
                }

                foreach ((string replicaGroupId, Protocol protocol) in _replicaGroups.Keys)
                {
                    if (protocol == Protocol.Ice1)
                    {
                        try
                        {
                            // This proxy is an indirect proxy with a location (the replica group ID).
                            IObjectPrx proxy = _dummyIce1Proxy.Clone(
                                IObjectPrx.Factory,
                                identity: identity,
                                location: ImmutableArray.Create(replicaGroupId));
                            proxy.IcePing();
                            return proxy;
                        }
                        catch
                        {
                            // Ignore and move on to the next replica group
                        }
                    }
                }

                foreach (string adapterId in _ice1Adapters.Keys)
                {
                    try
                    {
                        IObjectPrx proxy = _dummyIce1Proxy.Clone(
                            IObjectPrx.Factory,
                            identity: identity,
                            location: ImmutableArray.Create(adapterId));
                        proxy.IcePing();
                        return proxy;
                    }
                    catch
                    {
                        // Ignore.
                    }
                }

                return null;
            }
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

        internal (IReadOnlyList<EndpointData> Endpoints, string AdapterId) ResolveWellKnownProxy(Identity identity)
        {
            lock (_mutex)
            {
                if (identity.Name.Length == 0)
                {
                    return (ImmutableArray<EndpointData>.Empty, "");
                }

                foreach ((string replicaGroupId, Protocol protocol) in _replicaGroups.Keys)
                {
                    if (protocol == Protocol.Ice2)
                    {
                        try
                        {
                            // This proxy is an indirect proxy with a location (the replica group ID).
                            IObjectPrx proxy = _dummyIce2Proxy.Clone(
                                IObjectPrx.Factory,
                                identity: identity,
                                location: ImmutableArray.Create(replicaGroupId));
                            proxy.IcePing();
                            return (ImmutableArray<EndpointData>.Empty, replicaGroupId);
                        }
                        catch
                        {
                            // Ignore and move on to the next replica group
                        }
                    }
                }

                foreach (string adapterId in _ice2Adapters.Keys)
                {
                    try
                    {
                        IObjectPrx proxy = _dummyIce2Proxy.Clone(
                            IObjectPrx.Factory,
                            identity: identity,
                            location: ImmutableArray.Create(adapterId));
                        proxy.IcePing();
                        return (ImmutableArray<EndpointData>.Empty, adapterId);
                    }
                    catch
                    {
                        // Ignore.
                    }
                }

                return (ImmutableArray<EndpointData>.Empty, "");
            }
        }

        private void AddAdapterToReplicaGroup(string adapterId, string replicaGroupId, Protocol protocol)
        {
            // Must be called with _mutex locked.
            if (replicaGroupId.Length > 0)
            {
                if (!_replicaGroups.TryGetValue((replicaGroupId, protocol), out HashSet<string>? adapterIds))
                {
                    adapterIds = new HashSet<string>();
                    _replicaGroups.Add((replicaGroupId, protocol), adapterIds);
                }
                adapterIds.Add(adapterId);
            }
        }

        private void RemoveAdapterFromReplicaGroup(string adapterId, string replicaGroupId, Protocol protocol)
        {
            // Must be called with _mutex locked
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
