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
        setAdapterDirectProxyAsync(string adapterId, IObjectPrx proxy, Current current)
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
        setReplicatedAdapterDirectProxyAsync(string adapterId, string replicaGroupId, Ice.IObjectPrx proxy,
                                             Ice.Current current)
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
        setServerProcessProxyAsync(string id, Ice.ProcessPrx process, Ice.Current current)
        {
            return null;
        }

        internal Ice.IObjectPrx findObject(Ice.Identity id)
        {
            lock (this)
            {
                if (id.name.Length == 0)
                {
                    return null;
                }

                Ice.IObjectPrx prx = _wellKnownProxy.Clone(id);

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
                    foreach (KeyValuePair<string, Ice.IObjectPrx> entry in _adapters)
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
                }

                if (adapterIds.Count == 0)
                {
                    return null;
                }
                //adapterIds.Suffle();
                return prx.Clone(adapterId: adapterIds[0]);
            }
        }

        internal Ice.IObjectPrx findAdapter(string adapterId, out bool isReplicaGroup)
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
        public LocatorI(LookupI lookup, Ice.LocatorRegistryPrx registry)
        {
            _lookup = lookup;
            _registry = registry;
        }

        public Task<Ice.IObjectPrx>
        findObjectByIdAsync(Ice.Identity id, Ice.Current current)
        {
            return _lookup.findObject(id);
        }

        public Task<Ice.IObjectPrx>
        findAdapterByIdAsync(string adapterId, Ice.Current current)
        {
            return _lookup.findAdapter(adapterId);
        }

        public Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
        {
            return _registry;
        }

        private LookupI _lookup;
        private Ice.LocatorRegistryPrx _registry;
    };
}
