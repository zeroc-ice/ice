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
            _lookupServant.FindAdapterByIdAsync(adapterId, Protocol.Ice1, cancel);

        public ValueTask<IObjectPrx?> FindObjectByIdAsync(
            Identity id,
            Current current,
            CancellationToken cancel) =>
            _lookupServant.FindObjectByIdAsync(id, Protocol.Ice1, cancel);

        public ValueTask<ILocatorRegistryPrx?> GetRegistryAsync(Current current, CancellationToken cancel) =>
            new (_registry);

        public async ValueTask<(IEnumerable<EndpointData>, IEnumerable<string>)> ResolveLocationAsync(
            string[] location,
            Protocol protocol,
            Current current,
            CancellationToken cancel)
        {
            if ((byte)protocol < (byte)Protocol.Ice2)
            {
                // TODO: should we use & provide an ArgumentException-like remote exception?
                throw new ArgumentException("protocol must be ice2 or greater", nameof(protocol));
            }

            if (location.Length == 0)
            {
                throw new ArgumentException("location cannot be empty", nameof(location));
            }

            IObjectPrx? proxy =
                await _lookupServant.FindAdapterByIdAsync(location[0], protocol, cancel).ConfigureAwait(false);

            if (proxy == null)
            {
                return (ImmutableArray<EndpointData>.Empty, ImmutableArray<string>.Empty);
            }
            else
            {
                return (null!, location[1..]); // TODO: convert endpoints into endpoint data seq
            }
        }

        public async ValueTask<(IEnumerable<EndpointData>, IEnumerable<string>)> ResolveWellKnownProxyAsync(
            Identity identity,
            Protocol protocol,
            Current current,
            CancellationToken cancel)
        {
            // This implementation of ResolveWellKnownProxy pings the well-known proxy, so it only works with ice2,
            // not ice2 and up.
            if (protocol != Protocol.Ice2)
            {
                throw new ArgumentException("protocol must be ice2", nameof(protocol));
            }

            IObjectPrx? proxy =
                await _lookupServant.FindObjectByIdAsync(identity, protocol, cancel).ConfigureAwait(false);

            if (proxy == null)
            {
                return (ImmutableArray<EndpointData>.Empty, ImmutableArray<string>.Empty);
            }
            else
            {
                return (null!, proxy.Location); // TODO: convert endpoints into endpoint data seq
            }
        }

        internal Locator(Lookup lookupServant, ILocatorRegistryPrx registry)
        {
            _lookupServant = lookupServant;
            _registry = registry;
        }
    }

    internal class LocatorRegistry : ILocatorRegistry
    {
        private readonly IObjectPrx _dummyIce1Proxy;
        private readonly IObjectPrx _dummyIce2Proxy;

        private readonly Dictionary<string, IObjectPrx> _ice1Adapters = new ();

        // Dictionary for protocol >= ice2.
        private readonly Dictionary<(string, Protocol), IReadOnlyList<EndpointData>> _ice2Adapters = new ();

        private readonly object _mutex = new ();

        private readonly Dictionary<(string, Protocol), HashSet<string>> _replicaGroups = new ();

        public void RegisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            Protocol protocol,
            EndpointData[] endpoints,
            Current current,
            CancellationToken cancel)
        {
            if ((byte)protocol < (byte)Protocol.Ice2)
            {
                throw new ArgumentException("protocol must be ice2 or greater", nameof(protocol));
            }

            lock (_mutex)
            {
                try
                {
                    _ice2Adapters.Add((adapterId, protocol), endpoints);
                }
                catch (ArgumentException)
                {
                    throw new AdapterAlreadyActiveException($"adapter `{adapterId}' already has registered endpoints");
                }

                AddAdapterToReplicaGroup(replicaGroupId, adapterId, protocol);
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
            if ((byte)protocol < (byte)Protocol.Ice2)
            {
                throw new ArgumentException("protocol must be ice2 or greater", nameof(protocol));
            }

            lock (_mutex)
            {
                _ice2Adapters.Remove((adapterId, protocol));
                RemoveAdapterFromReplicaGroup(adapterId, replicaGroupId, protocol);
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

        internal IObjectPrx? FindObject(Identity identity, Protocol protocol)
        {
            lock (_mutex)
            {
                if (identity.Name.Length == 0)
                {
                    return null;
                }

                foreach ((string replicaGroupId, Protocol p) in _replicaGroups.Keys)
                {
                    if (protocol == p)
                    {
                        try
                        {
                            IObjectPrx dummyProxy = protocol == Protocol.Ice1 ? _dummyIce1Proxy : _dummyIce2Proxy;

                            // This proxy is an indirect proxy with a location (the replica group ID).
                            IObjectPrx proxy = dummyProxy.Clone(
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

                if (protocol == Protocol.Ice1)
                {
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
                }
                else
                {
                    foreach ((string adapterId, Protocol p) in _ice2Adapters.Keys)
                    {
                        if (p == Protocol.Ice2)
                        {
                            try
                            {
                                IObjectPrx proxy = _dummyIce2Proxy.Clone(
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
                    }
                }
                return null;
            }
        }

        internal (IReadOnlyList<EndpointData> Endpoints, bool IsReplicaGroup) ResolveLocation(
            string adapterId,
            Protocol protocol)
        {
            lock (_mutex)
            {
                if (_ice2Adapters.TryGetValue((adapterId, protocol), out IReadOnlyList<EndpointData>? endpoints))
                {
                    return (endpoints, false);
                }

                if (_replicaGroups.TryGetValue((adapterId, protocol), out HashSet<string>? adapterIds))
                {
                    Debug.Assert(adapterIds.Count > 0);

                    return (adapterIds.SelectMany(id => _ice2Adapters[(id, protocol)]).ToList(), true);
                }

                return (ImmutableArray<EndpointData>.Empty, false);
            }
        }
    }
}
