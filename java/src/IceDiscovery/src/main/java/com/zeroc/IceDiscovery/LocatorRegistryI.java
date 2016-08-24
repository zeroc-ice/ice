// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceDiscovery;

import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

class LocatorRegistryI implements com.zeroc.Ice.LocatorRegistry
{
    public LocatorRegistryI(com.zeroc.Ice.Communicator com)
    {
        _wellKnownProxy = com.stringToProxy("p").ice_locator(null).ice_router(null).ice_collocationOptimized(true);
    }

    @Override
    synchronized public CompletionStage<Void> setAdapterDirectProxyAsync(
        String adapterId,
        com.zeroc.Ice.ObjectPrx proxy,
        com.zeroc.Ice.Current current)
    {
        if(proxy != null)
        {
            _adapters.put(adapterId, proxy);
        }
        else
        {
            _adapters.remove(adapterId);
        }
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    synchronized public CompletionStage<Void> setReplicatedAdapterDirectProxyAsync(
        String adapterId,
        String replicaGroupId,
        com.zeroc.Ice.ObjectPrx proxy,
        com.zeroc.Ice.Current current)
    {
        if(proxy != null)
        {
            _adapters.put(adapterId, proxy);
            Set<String> s = _replicaGroups.get(replicaGroupId);
            if(s == null)
            {
                s = new HashSet<>();
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
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> setServerProcessProxyAsync(
        String serverId,
        com.zeroc.Ice.ProcessPrx process,
        com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    synchronized com.zeroc.Ice.ObjectPrx findObject(com.zeroc.Ice.Identity id)
    {
        if(id.name.length() == 0)
        {
            return null;
        }

        com.zeroc.Ice.ObjectPrx prx = _wellKnownProxy.ice_identity(id);

        List<String> adapterIds = new ArrayList<>();
        for(String a : _replicaGroups.keySet())
        {
            try
            {
                prx.ice_adapterId(a).ice_ping();
                adapterIds.add(a);
            }
            catch(com.zeroc.Ice.LocalException ex)
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
                catch(com.zeroc.Ice.LocalException ex)
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

    static class FindAdapterResult
    {
        com.zeroc.Ice.ObjectPrx returnValue;
        boolean isReplicaGroup;
    }

    synchronized FindAdapterResult findAdapter(String adapterId)
    {
        FindAdapterResult r = new FindAdapterResult();

        com.zeroc.Ice.ObjectPrx proxy = _adapters.get(adapterId);
        if(proxy != null)
        {
            r.isReplicaGroup = false;
            r.returnValue = proxy;
            return r;
        }

        Set<String> s = _replicaGroups.get(adapterId);
        if(s != null)
        {
            List<com.zeroc.Ice.Endpoint> endpoints = new ArrayList<>();
            com.zeroc.Ice.ObjectPrx prx = null;
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
                r.isReplicaGroup = true;
                r.returnValue = prx.ice_endpoints(endpoints.toArray(new com.zeroc.Ice.Endpoint[endpoints.size()]));
                return r;
            }
        }
        r.isReplicaGroup = false;
        r.returnValue = null;
        return r;
    }

    final com.zeroc.Ice.ObjectPrx _wellKnownProxy;
    final Map<String, com.zeroc.Ice.ObjectPrx> _adapters = new HashMap<>();
    final Map<String, Set<String>> _replicaGroups = new HashMap<>();
}
