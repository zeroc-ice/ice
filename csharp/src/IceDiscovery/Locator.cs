//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class Locator : ILocator
    {
        private readonly Lookup _lookup;
        private readonly ILocatorRegistryPrx _registry;

        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(string adapterId, Current current) =>
            _lookup.FindAdapterAsync(adapterId);

        public ValueTask<IObjectPrx?> FindObjectByIdAsync(Identity id, Current current) => _lookup.FindObjectAsync(id);

        public ILocatorRegistryPrx? GetRegistry(Current current) => _registry;

        internal Locator(Lookup lookup, ILocatorRegistryPrx registry)
        {
            _lookup = lookup;
            _registry = registry;
        }
    }

    internal class LocatorRegistry : ILocatorRegistry
    {
        private readonly Dictionary<string, IObjectPrx> _adapters = new Dictionary<string, IObjectPrx>();
        private readonly object _mutex = new object();
        private readonly Dictionary<string, HashSet<string>> _replicaGroups =
            new Dictionary<string, HashSet<string>>();
        private readonly IObjectPrx _wellKnownProxy;

        public LocatorRegistry(Communicator com) =>
            _wellKnownProxy = IObjectPrx.Parse("p", com).Clone(
                clearLocator: true, clearRouter: true, collocationOptimized: true);

        public ValueTask SetAdapterDirectProxyAsync(string adapterId, IObjectPrx? proxy, Current current)
        {
            lock (_mutex)
            {
                if (proxy != null)
                {
                    _adapters.Add(adapterId, proxy);
                }
                else
                {
                    _adapters.Remove(adapterId);
                }
            }
            return new ValueTask();
        }

        public ValueTask SetReplicatedAdapterDirectProxyAsync(string adapterId, string replicaGroupId, IObjectPrx? proxy,
            Current current)
        {
            lock (_mutex)
            {
                HashSet<string>? adapterIds;
                if (proxy != null)
                {
                    _adapters.Add(adapterId, proxy);
                    if (!_replicaGroups.TryGetValue(replicaGroupId, out adapterIds))
                    {
                        adapterIds = new HashSet<string>();
                        _replicaGroups.Add(replicaGroupId, adapterIds);
                    }
                    adapterIds.Add(adapterId);
                }
                else
                {
                    _adapters.Remove(adapterId);
                    if (_replicaGroups.TryGetValue(replicaGroupId, out adapterIds))
                    {
                        adapterIds.Remove(adapterId);
                        if (adapterIds.Count == 0)
                        {
                            _replicaGroups.Remove(replicaGroupId);
                        }
                    }
                }
            }
            return new ValueTask();
        }

        public ValueTask SetServerProcessProxyAsync(string id, IProcessPrx? process, Current current) =>
            new ValueTask(Task.CompletedTask);

        internal (IObjectPrx? Proxy, bool IsReplicaGroup) FindAdapter(string adapterId)
        {
            lock (_mutex)
            {
                if (_adapters.TryGetValue(adapterId, out IObjectPrx? result))
                {
                    return (result, false);
                }

                if (_replicaGroups.TryGetValue(adapterId, out HashSet<string>? adapterIds))
                {
                    var endpoints = new List<Endpoint>();
                    foreach (string a in adapterIds)
                    {
                        if (!_adapters.TryGetValue(a, out IObjectPrx? proxy))
                        {
                            continue; // TODO: Inconsistency
                        }

                        if (result == null)
                        {
                            result = proxy;
                        }

                        endpoints.AddRange(proxy.Endpoints);
                    }

                    if (result != null)
                    {
                        return (result.Clone(endpoints: endpoints), true);
                    }
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

                IObjectPrx prx = _wellKnownProxy.Clone(identity, IObjectPrx.Factory);

                var adapterIds = new List<string>();
                foreach (KeyValuePair<string, HashSet<string>> entry in _replicaGroups)
                {
                    try
                    {
                        prx.Clone(adapterId: entry.Key).IcePing();
                        adapterIds.Add(entry.Key);
                    }
                    catch
                    {
                        // Ignore.
                    }
                }
                if (adapterIds.Count == 0)
                {
                    foreach (KeyValuePair<string, IObjectPrx> entry in _adapters)
                    {
                        try
                        {
                            prx.Clone(adapterId: entry.Key).IcePing();
                            adapterIds.Add(entry.Key);
                        }
                        catch
                        {
                            // Ignore.
                        }
                    }
                }

                if (adapterIds.Count == 0)
                {
                    return null;
                }

                return prx.Clone(adapterId: adapterIds.Shuffle().First());
            }
        }
    }
}
