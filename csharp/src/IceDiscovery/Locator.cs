//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceDiscovery
{
    using System.Collections.Generic;
    using System.Threading.Tasks;
    using Ice;

    internal class LocatorRegistry : ILocatorRegistry
    {
        public
        LocatorRegistry(Communicator com) =>
            _wellKnownProxy = IObjectPrx.Parse("p", com).Clone(
                clearLocator: true, clearRouter: true, collocationOptimized: true);

        public Task
        SetAdapterDirectProxyAsync(string adapterId, IObjectPrx proxy, Current current)
        {
            lock (this)
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
            return null;
        }

        public Task
        SetReplicatedAdapterDirectProxyAsync(string adapterId, string replicaGroupId, IObjectPrx proxy,
                                             Current current)
        {
            lock (this)
            {
                if (proxy != null)
                {
                    _adapters.Add(adapterId, proxy);
                    HashSet<string> adapterIds;
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
                    HashSet<string> adapterIds;
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
            return null;
        }

        public Task
        SetServerProcessProxyAsync(string id, IProcessPrx process, Current current) => null;

        internal IObjectPrx FindObject(Identity id)
        {
            lock (this)
            {
                if (id.Name.Length == 0)
                {
                    return null;
                }

                IObjectPrx prx = _wellKnownProxy.Clone(id);

                List<string> adapterIds = new List<string>();
                foreach (KeyValuePair<string, HashSet<string>> entry in _replicaGroups)
                {
                    try
                    {
                        prx.Clone(adapterId: entry.Key).IcePing();
                        adapterIds.Add(entry.Key);
                    }
                    catch (Exception)
                    {
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
                        catch (Exception)
                        {
                        }
                    }
                }

                if (adapterIds.Count == 0)
                {
                    return null;
                }
                //adapterIds.Suffle();
                return prx.Clone(adapterId: adapterIds[0]);
            }
        }

        internal IObjectPrx FindAdapter(string adapterId, out bool isReplicaGroup)
        {
            lock (this)
            {
                IObjectPrx result = null;
                if (_adapters.TryGetValue(adapterId, out result))
                {
                    isReplicaGroup = false;
                    return result;
                }

                HashSet<string> adapterIds;
                if (_replicaGroups.TryGetValue(adapterId, out adapterIds))
                {
                    List<IEndpoint> endpoints = new List<IEndpoint>();
                    foreach (string a in adapterIds)
                    {
                        IObjectPrx proxy;
                        if (!_adapters.TryGetValue(a, out proxy))
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
                        isReplicaGroup = true;
                        return result.Clone(endpoints: endpoints.ToArray());
                    }
                }

                isReplicaGroup = false;
                return null;
            }
        }

        private readonly IObjectPrx _wellKnownProxy;
        private Dictionary<string, IObjectPrx> _adapters = new Dictionary<string, IObjectPrx>();
        private Dictionary<string, HashSet<string>> _replicaGroups = new Dictionary<string, HashSet<string>>();
    };

    internal class Locator : ILocator
    {
        public Locator(Lookup lookup, ILocatorRegistryPrx registry)
        {
            _lookup = lookup;
            _registry = registry;
        }

        public Task<IObjectPrx>
        FindObjectByIdAsync(Identity id, Current current) => _lookup.FindObject(id);

        public Task<IObjectPrx>
        FindAdapterByIdAsync(string adapterId, Current current) => _lookup.FindAdapter(adapterId);

        public ILocatorRegistryPrx GetRegistry(Current current) => _registry;

        private Lookup _lookup;
        private ILocatorRegistryPrx _registry;
    };
}
