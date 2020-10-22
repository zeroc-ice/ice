// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    /// <summary>Servant class that implements the Slice interface Ice::LocatorRegistry.</summary>
    internal class LocatorRegistry : ILocatorRegistry
    {
        private readonly IObjectPrx _dummyIce1Proxy;
        private readonly IObjectPrx _dummyIce2Proxy;

        private readonly Dictionary<string, IObjectPrx> _ice1Adapters = new ();
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
            if (adapterId.Length == 0)
            {
                throw new InvalidArgumentException("adapterId cannot be empty", nameof(adapterId));
            }
            if (endpoints.Length == 0)
            {
                throw new InvalidArgumentException("endpoints cannot be empty", nameof(endpoints));
            }

            lock (_mutex)
            {
                _ice2Adapters[adapterId] = endpoints;
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
                    _ice1Adapters[adapterId] = proxy;
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
            if (adapterId.Length == 0)
            {
                throw new InvalidArgumentException("adapterId cannot be empty", nameof(adapterId));
            }

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
                    var endpoints = adapterIds.SelectMany(id => _ice1Adapters[id].Endpoints).ToList();
                    return (_dummyIce1Proxy.Clone(endpoints: endpoints), true);
                }

                return (null, false);
            }
        }

        internal IObjectPrx? FindObject(Identity identity)
        {
            if (identity.Name.Length == 0)
            {
                return null;
            }

            lock (_mutex)
            {
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

        internal string ResolveWellKnownProxy(Identity identity)
        {
            if (identity.Name.Length == 0)
            {
                return "";
            }

            lock (_mutex)
            {
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
                            return replicaGroupId;
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
                        return adapterId;
                    }
                    catch
                    {
                        // Ignore.
                    }
                }

                return "";
            }
        }

        private void AddAdapterToReplicaGroup(string adapterId, string replicaGroupId, Protocol protocol)
        {
            // Must be called with _mutex locked.
            if (replicaGroupId.Length > 0)
            {
                if (!_replicaGroups.TryGetValue((replicaGroupId, protocol), out HashSet<string>? adapterIds))
                {
                    adapterIds = new ();
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
