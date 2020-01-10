//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceDiscovery
{
    using System.Collections.Generic;
    using System.Threading.Tasks;
    using Ice;

    internal class LocatorRegistryI : Ice.LocatorRegistry
    {
        public
        LocatorRegistryI(Communicator com)
        {
            _wellKnownProxy = IObjectPrx.Parse("p", com).Clone(clearLocator: true,
                                                                  clearRouter: true,
                                                                  collocationOptimized: true);
        }

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
        SetServerProcessProxyAsync(string id, IProcessPrx process, Current current)
        {
            return null;
        }

        internal IObjectPrx FindObject(Identity id)
        {
            lock (this)
            {
                if (id.name.Length == 0)
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
                    catch (Ice.Exception)
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

        internal Ice.IObjectPrx FindAdapter(string adapterId, out bool isReplicaGroup)
        {
            lock (this)
            {
                Ice.IObjectPrx result = null;
                if (_adapters.TryGetValue(adapterId, out result))
                {
                    isReplicaGroup = false;
                    return result;
                }

                HashSet<string> adapterIds;
                if (_replicaGroups.TryGetValue(adapterId, out adapterIds))
                {
                    List<Ice.Endpoint> endpoints = new List<Ice.Endpoint>();
                    foreach (string a in adapterIds)
                    {
                        Ice.IObjectPrx proxy;
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

        private readonly Ice.IObjectPrx _wellKnownProxy;
        private Dictionary<string, Ice.IObjectPrx> _adapters = new Dictionary<string, Ice.IObjectPrx>();
        private Dictionary<string, HashSet<string>> _replicaGroups = new Dictionary<string, HashSet<string>>();
    };

    internal class LocatorI : Ice.Locator
    {
        public LocatorI(LookupI lookup, Ice.ILocatorRegistryPrx registry)
        {
            _lookup = lookup;
            _registry = registry;
        }

        public Task<Ice.IObjectPrx>
        FindObjectByIdAsync(Ice.Identity id, Ice.Current current)
        {
            return _lookup.findObject(id);
        }

        public Task<Ice.IObjectPrx>
        FindAdapterByIdAsync(string adapterId, Current current)
        {
            return _lookup.FindAdapter(adapterId);
        }

        public Ice.ILocatorRegistryPrx GetRegistry(Current current)
        {
            return _registry;
        }

        private LookupI _lookup;
        private Ice.ILocatorRegistryPrx _registry;
    };
}
