// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceDiscovery
{
    using System;
    using System.Collections.Generic;

    class LocatorRegistryI : Ice.LocatorRegistryDisp_
    {
        public 
        LocatorRegistryI(Ice.Communicator com)
        {
            _wellKnownProxy = com.stringToProxy("p").ice_locator(null).ice_router(null).ice_collocationOptimized(true);
        }

        public override void 
        setAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setAdapterDirectProxy cb, 
                                    string adapterId, 
                                    Ice.ObjectPrx proxy, 
                                    Ice.Current current)
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
                cb.ice_response();
            }
        }

        public override void
        setReplicatedAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setReplicatedAdapterDirectProxy cb,
                                              string adapterId,
                                              string replicaGroupId,
                                              Ice.ObjectPrx proxy, 
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
            cb.ice_response();
        }

        public override void
        setServerProcessProxy_async(Ice.AMD_LocatorRegistry_setServerProcessProxy cb, 
                                    string id, 
                                    Ice.ProcessPrx process,
                                    Ice.Current current)
        {
            cb.ice_response();
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

        public override void findObjectById_async(Ice.AMD_Locator_findObjectById cb, Ice.Identity id, Ice.Current c)
        {
            _lookup.findObject(cb, id);
        }

        public override void findAdapterById_async(Ice.AMD_Locator_findAdapterById cb, string adapterId, Ice.Current c)
        {
            _lookup.findAdapter(cb, adapterId);
        }

        public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
        {
            return _registry;
        }
 
        private LookupI _lookup;
        private Ice.LocatorRegistryPrx _registry;
   };
}
