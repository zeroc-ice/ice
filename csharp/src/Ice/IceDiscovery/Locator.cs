// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Diagnostics;
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

        public ValueTask SetAdapterDirectProxyAsync(string adapterId, IObjectPrx? proxy, Current current)
        {
            lock (_mutex)
            {
                if (proxy != null)
                {
                    _adapters[adapterId] = proxy.Clone(clearLocator: true, clearRouter: true);
                }
                else
                {
                    _adapters.Remove(adapterId);
                }
            }
            return new ValueTask();
        }

        public ValueTask SetReplicatedAdapterDirectProxyAsync(
            string adapterId,
            string replicaGroupId,
            IObjectPrx? proxy,
            Current current)
        {
            lock (_mutex)
            {
                HashSet<string>? adapterIds;
                if (proxy != null)
                {
                    if (_replicaGroups.TryGetValue(replicaGroupId, out adapterIds))
                    {
                        if (_adapters.TryGetValue(adapterIds.First(), out IObjectPrx? registeredProxy) &&
                            registeredProxy.Protocol != proxy.Protocol)
                        {
                            throw new InvalidProxyException(
                                $"The proxy protocol {proxy.Protocol} doesn't match the replica group protocol");
                        }
                    }
                    else
                    {
                        adapterIds = new HashSet<string>();
                        _replicaGroups.Add(replicaGroupId, adapterIds);
                    }
                    _adapters[adapterId] = proxy.Clone(clearLocator: true, clearRouter: true);
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
            return default;
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
                    Debug.Assert(adapterIds.Count > 0);
                    foreach (string id in adapterIds)
                    {
                        if (!_adapters.TryGetValue(id, out IObjectPrx? proxy))
                        {
                            continue; // TODO: Inconsistency
                        }
                        result ??= proxy;

                        endpoints.AddRange(proxy.Endpoints);
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

                foreach ((string key, HashSet<string> ids) in _replicaGroups)
                {
                    try
                    {
                        IObjectPrx proxy = _adapters[ids.First()];
                        proxy = proxy.Clone(IObjectPrx.Factory, adapterId: key, identity: identity);
                        proxy.IcePing();
                        return proxy;
                    }
                    catch
                    {
                        // Ignore.
                    }
                }

                foreach ((string key, IObjectPrx registeredProxy) in _adapters)
                {
                    try
                    {
                        IObjectPrx proxy = registeredProxy.Clone(IObjectPrx.Factory,
                                                                 adapterId: key,
                                                                 identity: identity);
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
    }
}
