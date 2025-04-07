// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.LocatorRegistry;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.ProcessPrx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

class LocatorRegistryI implements LocatorRegistry {
    public LocatorRegistryI(Communicator com) {
        _wellKnownProxy =
                com.stringToProxy("p")
                        .ice_locator(null)
                        .ice_router(null)
                        .ice_collocationOptimized(true);
    }

    @Override
    public synchronized CompletionStage<Void> setAdapterDirectProxyAsync(
            String adapterId, ObjectPrx proxy, Current current) {
        if (proxy != null) {
            _adapters.put(adapterId, proxy);
        } else {
            _adapters.remove(adapterId);
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public synchronized CompletionStage<Void> setReplicatedAdapterDirectProxyAsync(
            String adapterId,
            String replicaGroupId,
            ObjectPrx proxy,
            Current current) {
        if (proxy != null) {
            _adapters.put(adapterId, proxy);
            Set<String> s = _replicaGroups.get(replicaGroupId);
            if (s == null) {
                s = new HashSet<>();
                _replicaGroups.put(replicaGroupId, s);
            }
            s.add(adapterId);
        } else {
            _adapters.remove(adapterId);
            Set<String> s = _replicaGroups.get(replicaGroupId);
            if (s != null) {
                s.remove(adapterId);
                if (s.isEmpty()) {
                    _replicaGroups.remove(adapterId);
                }
            }
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> setServerProcessProxyAsync(
            String serverId, ProcessPrx process, Current current) {
        return CompletableFuture.completedFuture((Void) null);
    }

    synchronized ObjectPrx findObject(Identity id) {
        if (id.name.isEmpty()) {
            return null;
        }

        ObjectPrx prx = _wellKnownProxy.ice_identity(id);

        List<String> adapterIds = new ArrayList<>();
        for (String a : _replicaGroups.keySet()) {
            try {
                prx.ice_adapterId(a).ice_ping();
                adapterIds.add(a);
            } catch (LocalException ex) {
            }
        }
        if (adapterIds.isEmpty()) {
            for (String a : _adapters.keySet()) {
                try {
                    prx.ice_adapterId(a).ice_ping();
                    adapterIds.add(a);
                } catch (LocalException ex) {
                }
            }
        }

        if (adapterIds.isEmpty()) {
            return null;
        }
        Collections.shuffle(adapterIds);
        return prx.ice_adapterId(adapterIds.get(0));
    }

    static class FindAdapterResult {
        ObjectPrx returnValue;
        boolean isReplicaGroup;
    }

    synchronized FindAdapterResult findAdapter(String adapterId) {
        FindAdapterResult r = new FindAdapterResult();

        ObjectPrx proxy = _adapters.get(adapterId);
        if (proxy != null) {
            r.isReplicaGroup = false;
            r.returnValue = proxy;
            return r;
        }

        Set<String> s = _replicaGroups.get(adapterId);
        if (s != null) {
            List<Endpoint> endpoints = new ArrayList<>();
            ObjectPrx prx = null;
            for (String a : s) {
                proxy = _adapters.get(a);
                if (proxy == null) {
                    continue; // TODO: Inconsistency
                }

                if (prx == null) {
                    prx = proxy;
                }

                endpoints.addAll(Arrays.asList(proxy.ice_getEndpoints()));
            }

            if (prx != null) {
                r.isReplicaGroup = true;
                r.returnValue =
                        prx.ice_endpoints(
                                endpoints.toArray(new Endpoint[endpoints.size()]));
                return r;
            }
        }
        r.isReplicaGroup = false;
        r.returnValue = null;
        return r;
    }

    final ObjectPrx _wellKnownProxy;
    final Map<String, ObjectPrx> _adapters = new HashMap<>();
    final Map<String, Set<String>> _replicaGroups = new HashMap<>();
}
