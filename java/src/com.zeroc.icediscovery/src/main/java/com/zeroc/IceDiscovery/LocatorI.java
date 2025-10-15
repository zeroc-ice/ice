// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.AsyncLocator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.ObjectPrx;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

class LocatorI implements AsyncLocator {
    public LocatorI(LookupI lookup, LocatorRegistryPrx registry) {
        _lookup = lookup;
        _registry = registry;
    }

    @Override
    public CompletionStage<ObjectPrx> findObjectByIdAsync(Identity id, Current current) {
        CompletableFuture<ObjectPrx> f = new CompletableFuture<ObjectPrx>();
        _lookup.findObject(f, id);
        return f;
    }

    @Override
    public CompletionStage<ObjectPrx> findAdapterByIdAsync(String adapterId, Current current) {
        CompletableFuture<ObjectPrx> f = new CompletableFuture<ObjectPrx>();
        _lookup.findAdapter(f, adapterId);
        return f;
    }

    @Override
    public CompletionStage<LocatorRegistryPrx> getRegistryAsync(Current current) {
        return CompletableFuture.completedFuture(_registry);
    }

    private final LookupI _lookup;
    private final LocatorRegistryPrx _registry;
}
