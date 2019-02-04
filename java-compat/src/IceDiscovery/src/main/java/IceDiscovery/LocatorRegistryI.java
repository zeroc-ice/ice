//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceDiscovery;

import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

class LocatorRegistryI extends Ice._LocatorRegistryDisp
{
    public LocatorRegistryI(Ice.Communicator com)
    {
        _wellKnownProxy = com.stringToProxy("p").ice_locator(null).ice_router(null).ice_collocationOptimized(true);
    }

    @Override
    synchronized public void
    setAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setAdapterDirectProxy cb,
                                String adapterId,
                                Ice.ObjectPrx proxy,
                                Ice.Current current)
    {
        if(proxy != null)
        {
            _adapters.put(adapterId, proxy);
        }
        else
        {
            _adapters.remove(adapterId);
        }
        cb.ice_response();
    }

    @Override
    synchronized public void
    setReplicatedAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setReplicatedAdapterDirectProxy cb,
                                          String adapterId,
                                          String replicaGroupId,
                                          Ice.ObjectPrx proxy,
                                          Ice.Current current)
    {
        if(proxy != null)
        {
            _adapters.put(adapterId, proxy);
            Set<String> s = _replicaGroups.get(replicaGroupId);
            if(s == null)
            {
                s = new HashSet<String>();
                _replicaGroups.put(replicaGroupId, s);
            }
            s.add(adapterId);
        }
        else
        {
            _adapters.remove(adapterId);
            Set<String> s = _replicaGroups.get(replicaGroupId);
            if(s != null)
            {
                s.remove(adapterId);
                if(s.isEmpty())
                {
                    _replicaGroups.remove(adapterId);
                }
            }
        }
        cb.ice_response();
    }

    @Override
    public void
    setServerProcessProxy_async(Ice.AMD_LocatorRegistry_setServerProcessProxy cb,
                                String serverId,
                                Ice.ProcessPrx process,
                                Ice.Current current)
    {
        cb.ice_response();
    }

    synchronized Ice.ObjectPrx
    findObject(Ice.Identity id)
    {
        if(id.name.length() == 0)
        {
            return null;
        }

        Ice.ObjectPrx prx = _wellKnownProxy.ice_identity(id);

        List<String> adapterIds = new ArrayList<String>();
        for(String a : _replicaGroups.keySet())
        {
            try
            {
                prx.ice_adapterId(a).ice_ping();
                adapterIds.add(a);
            }
            catch(Ice.LocalException ex)
            {
            }
        }
        if(adapterIds.isEmpty())
        {
            for(String a : _adapters.keySet())
            {
                try
                {
                    prx.ice_adapterId(a).ice_ping();
                    adapterIds.add(a);
                }
                catch(Ice.LocalException ex)
                {
                }
            }
        }

        if(adapterIds.isEmpty())
        {
            return null;
        }
        java.util.Collections.shuffle(adapterIds);
        return prx.ice_adapterId(adapterIds.get(0));
    }

    synchronized Ice.ObjectPrx
    findAdapter(String adapterId, Ice.Holder<Boolean> isReplicaGroup)
    {
        Ice.ObjectPrx proxy = _adapters.get(adapterId);
        if(proxy != null)
        {
            isReplicaGroup.value = false;
            return proxy;
        }

        Set<String> s = _replicaGroups.get(adapterId);
        if(s != null)
        {
            List<Ice.Endpoint> endpoints = new ArrayList<Ice.Endpoint>();
            Ice.ObjectPrx prx = null;
            for(String a : s)
            {
                proxy = _adapters.get(a);
                if(proxy == null)
                {
                    continue; // TODO: Inconsistency
                }

                if(prx == null)
                {
                    prx = proxy;
                }

                endpoints.addAll(java.util.Arrays.asList(proxy.ice_getEndpoints()));
            }

            if(prx != null)
            {
                isReplicaGroup.value = true;
                return prx.ice_endpoints(endpoints.toArray(new Ice.Endpoint[endpoints.size()]));
            }
        }
        isReplicaGroup.value = false;
        return null;
    }

    final Ice.ObjectPrx _wellKnownProxy;
    final Map<String, Ice.ObjectPrx> _adapters = new HashMap<String, Ice.ObjectPrx>();
    final Map<String, Set<String>> _replicaGroups = new HashMap<String, Set<String>>();
}
