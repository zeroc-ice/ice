// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.Locator;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.ObjectPrx;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

class LocatorI implements Locator {
    public LocatorI(LookupI lookup, LocatorRegistryPrx registry) {
        _lookup = lookup;
        _registry = registry;
    }

    @Override
    public CompletionStage<ObjectPrx> findObjectByIdAsync(
            Identity id, Current current) {
        CompletableFuture<ObjectPrx> f =
            new CompletableFuture<ObjectPrx>();
        _lookup.findObject(f, id);
        return f;
    }

    @Override
    public CompletionStage<ObjectPrx> findAdapterByIdAsync(
            String adapterId, Current current) {
        CompletableFuture<ObjectPrx> f =
            new CompletableFuture<ObjectPrx>();
        _lookup.findAdapter(f, adapterId);
        return f;
    }

    @Override
    public LocatorRegistryPrx getRegistry(Current current) {
        return _registry;
    }

    private final LookupI _lookup;
    private final LocatorRegistryPrx _registry;
}
