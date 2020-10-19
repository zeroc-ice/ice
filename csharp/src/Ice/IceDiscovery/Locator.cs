// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class Locator : IAsyncLocator
    {
        private readonly Lookup _lookupServant;
        private readonly ILocatorRegistryPrx _registry;

        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(
            string adapterId,
            Current current,
            CancellationToken cancel) =>
            ResolveLocationAsync(new string[] { adapterId }, Protocol.Ice1, current, cancel);

        public ValueTask<IObjectPrx?> FindObjectByIdAsync(
            Identity id,
            Current current,
            CancellationToken cancel) =>
            ResolveWellKnownProxyAsync(id, Protocol.Ice1, current, cancel);

        public ValueTask<ILocatorRegistryPrx?> GetRegistryAsync(Current current, CancellationToken cancel) =>
            new (_registry);

        public ValueTask<IObjectPrx?> ResolveLocationAsync(
            string[] location,
            Protocol protocol,
            Current current,
            CancellationToken cancel) =>
            _lookupServant.FindAdapterByIdAsync(location, protocol);

        public ValueTask<IObjectPrx?> ResolveWellKnownProxyAsync(
            Identity identity,
            Protocol protocol,
            Current current,
            CancellationToken cancel) =>
            _lookupServant.FindObjectByIdAsync(identity, protocol);

        internal Locator(Lookup lookupServant, ILocatorRegistryPrx registry)
        {
            _lookupServant = lookupServant;
            _registry = registry;
        }
    }

    internal class LocatorRegistry : ILocatorRegistry
    {
        private readonly Dictionary<(string, Protocol), IObjectPrx> _adapters = new ();
        private readonly object _mutex = new ();
        private readonly Dictionary<(string, Protocol), HashSet<string>> _replicaGroups = new ();

        public void RegisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            IObjectPrx proxy,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                Protocol protocol = proxy.Protocol;

                try
                {
                    _adapters.Add((adapterId, protocol), proxy.Clone(clearLocator: true, clearRouter: true));
                }
                catch (ArgumentException)
                {
                    throw new AdapterAlreadyActiveException($"adapter `{adapterId}' already has registered endpoints");
                }

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
        }

        public void SetAdapterDirectProxy(
            string adapterId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel)
        {
            Debug.Assert(false); // this method is never called since this servant is hosted by an ice2 object adapter.
        }

        public void SetReplicatedAdapterDirectProxy(
           string adapterId,
           string replicaGroupId,
           IObjectPrx? proxy,
           Current current,
           CancellationToken cancel)
        {
            Debug.Assert(false); // this method is never called since this servant is hosted by an ice2 object adapter.
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
            Protocol protocol,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                _adapters.Remove((adapterId, protocol));

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

        internal (IObjectPrx? Proxy, bool IsReplicaGroup) FindAdapter(string adapterId, Protocol protocol)
        {
            lock (_mutex)
            {
                if (_adapters.TryGetValue((adapterId, protocol), out IObjectPrx? proxy))
                {
                    return (proxy, false);
                }

                if (_replicaGroups.TryGetValue((adapterId, protocol), out HashSet<string>? adapterIds))
                {
                    Debug.Assert(adapterIds.Count > 0);

                    IObjectPrx? result = null;
                    var endpoints = new List<Endpoint>();
                    foreach (string id in adapterIds)
                    {
                        // We assume that if adapterId is in adapterIds, it's also in the _adapters dictionary. The two
                        // dictionaries can be out of sync if there is a bug in the local Ice runtime or application
                        // code that uses this local colocated LocatorRegistry directly. For example, a call to
                        // unregisterAdapterEndpoints with a missing or incorrect replicaGroupId. If there is such a
                        // local bug, the code will throw a KeyNotFoundException.
                        IObjectPrx p = _adapters[(id, protocol)];
                        result ??= p;
                        endpoints.AddRange(p.Endpoints);
                    }

                    return (result?.Clone(endpoints: endpoints), result != null);
                }

                return (null, false);
            }
        }

        internal IObjectPrx? FindObject(Identity identity, Protocol protocol)
        {
            lock (_mutex)
            {
                if (identity.Name.Length == 0)
                {
                    return null;
                }

                foreach (((string ReplicaGroupId, Protocol Protocol) key, HashSet<string> adapterIds)
                    in _replicaGroups)
                {
                    if (key.Protocol == protocol)
                    {
                        // We retrieve this proxy only for its protocol - everything else gets replaced by the Clone
                        // below.
                        IObjectPrx dummy = _adapters[(adapterIds.First(), protocol)];
                        try
                        {
                            // This proxy is an indirect proxy with a location (the replica group ID).
                            IObjectPrx proxy = dummy.Clone(IObjectPrx.Factory,
                                                           endpoints: ImmutableArray<Endpoint>.Empty,
                                                           identity: identity,
                                                           location: ImmutableArray.Create(key.ReplicaGroupId));
                            proxy.IcePing();
                            return proxy;
                        }
                        catch
                        {
                            // Ignore and move on to the next replica group
                        }
                    }

                }

                foreach (((string AdapterId, Protocol Protocol) key, IObjectPrx dummy) in _adapters)
                {
                    if (key.Protocol == protocol)
                    {
                        try
                        {
                            IObjectPrx proxy = dummy.Clone(IObjectPrx.Factory,
                                                           endpoints: ImmutableArray<Endpoint>.Empty,
                                                           identity: identity,
                                                           location: ImmutableArray.Create(key.AdapterId));
                            proxy.IcePing();
                            return proxy;
                        }
                        catch
                        {
                            // Ignore.
                        }
                    }
                }

                return null;
            }
        }
    }
}
