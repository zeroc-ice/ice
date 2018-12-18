// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace IceDiscovery
{
    using System.Collections.Generic;
    using System.Threading.Tasks;

    class LocatorRegistryI : Ice.LocatorRegistryDisp_
    {
        public
        LocatorRegistryI(Ice.Communicator com)
        {
            _wellKnownProxy = com.stringToProxy("p").ice_locator(null).ice_router(null).ice_collocationOptimized(true);
        }

        public override Task
        setAdapterDirectProxyAsync(string adapterId, Ice.ObjectPrx proxy, Ice.Current current)
        {
            lock(this)
            {
                if(proxy != null)
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

        public override Task
        setReplicatedAdapterDirectProxyAsync(string adapterId, string replicaGroupId, Ice.ObjectPrx proxy,
                                             Ice.Current current)
        {
            lock(this)
            {
                if(proxy != null)
                {
                    _adapters.Add(adapterId, proxy);
                    HashSet<string> adapterIds;
                    if(!_replicaGroups.TryGetValue(replicaGroupId, out adapterIds))
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
                    if(_replicaGroups.TryGetValue(replicaGroupId, out adapterIds))
                    {
                        adapterIds.Remove(adapterId);
                        if(adapterIds.Count == 0)
                        {
                            _replicaGroups.Remove(replicaGroupId);
                        }
                    }
                }
            }
            return null;
        }

        public override Task
        setServerProcessProxyAsync(string id, Ice.ProcessPrx process, Ice.Current current)
        {
            return null;
        }

        internal Ice.ObjectPrx findObject(Ice.Identity id)
        {
            lock(this)
            {
                if(id.name.Length == 0)
                {
                    return null;
                }

                Ice.ObjectPrx prx = _wellKnownProxy.ice_identity(id);

                List<string> adapterIds = new List<string>();
                foreach(KeyValuePair<string, HashSet<string>> entry in _replicaGroups)
                {
                    try
                    {
                        prx.ice_adapterId(entry.Key).ice_ping();
                        adapterIds.Add(entry.Key);
                    }
                    catch(Ice.Exception)
                    {
                    }
                }
                if(adapterIds.Count == 0)
                {
                    foreach(KeyValuePair<string, Ice.ObjectPrx> entry in _adapters)
                    {
                        try
                        {
                            prx.ice_adapterId(entry.Key).ice_ping();
                            adapterIds.Add(entry.Key);
                        }
                        catch(Ice.Exception)
                        {
                        }
                    }
                }

                if(adapterIds.Count == 0)
                {
                    return null;
                }
                //adapterIds.Suffle();
                return prx.ice_adapterId(adapterIds[0]);
            }
        }

        internal Ice.ObjectPrx findAdapter(string adapterId, out bool isReplicaGroup)
        {
            lock(this)
            {
                Ice.ObjectPrx result = null;
                if(_adapters.TryGetValue(adapterId, out result))
                {
                    isReplicaGroup = false;
                    return result;
                }

                HashSet<string> adapterIds;
                if(_replicaGroups.TryGetValue(adapterId, out adapterIds))
                {
                    List<Ice.Endpoint> endpoints = new List<Ice.Endpoint>();
                    foreach(string a in adapterIds)
                    {
                        Ice.ObjectPrx proxy;
                        if(!_adapters.TryGetValue(a, out proxy))
                        {
                            continue; // TODO: Inconsistency
                        }

                        if(result == null)
                        {
                            result = proxy;
                        }

                        endpoints.AddRange(proxy.ice_getEndpoints());
                    }

                    if(result != null)
                    {
                        isReplicaGroup = true;
                        return result.ice_endpoints(endpoints.ToArray());
                    }
                }

                isReplicaGroup = false;
                return null;
            }
        }

        private readonly Ice.ObjectPrx _wellKnownProxy;
        private Dictionary<string, Ice.ObjectPrx> _adapters = new Dictionary<string, Ice.ObjectPrx>();
        private Dictionary<string, HashSet<string>> _replicaGroups = new Dictionary<string, HashSet<string>>();
    };

    class LocatorI : Ice.LocatorDisp_
    {
        public LocatorI(LookupI lookup, Ice.LocatorRegistryPrx registry)
        {
            _lookup = lookup;
            _registry = registry;
        }

        public override Task<Ice.ObjectPrx>
        findObjectByIdAsync(Ice.Identity id, Ice.Current current)
        {
            return _lookup.findObject(id);
        }

        public override Task<Ice.ObjectPrx>
        findAdapterByIdAsync(string adapterId, Ice.Current current)
        {
            return _lookup.findAdapter(adapterId);
        }

        public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
        {
            return _registry;
        }

        private LookupI _lookup;
        private Ice.LocatorRegistryPrx _registry;
   };
}
