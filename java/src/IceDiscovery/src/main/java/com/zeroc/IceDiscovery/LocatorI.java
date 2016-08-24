// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceDiscovery;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

class LocatorI implements com.zeroc.Ice.Locator
{
    public LocatorI(LookupI lookup, com.zeroc.Ice.LocatorRegistryPrx registry)
    {
        _lookup = lookup;
        _registry = registry;
    }

    @Override
    public CompletionStage<com.zeroc.Ice.ObjectPrx> findObjectByIdAsync(com.zeroc.Ice.Identity id,
                                                                        com.zeroc.Ice.Current current)
    {
        CompletableFuture<com.zeroc.Ice.ObjectPrx> f = new CompletableFuture<com.zeroc.Ice.ObjectPrx>();
        _lookup.findObject(f, id);
        return f;
    }

    @Override
    public CompletionStage<com.zeroc.Ice.ObjectPrx> findAdapterByIdAsync(String adapterId,
                                                                         com.zeroc.Ice.Current current)
    {
        CompletableFuture<com.zeroc.Ice.ObjectPrx> f = new CompletableFuture<com.zeroc.Ice.ObjectPrx>();
        _lookup.findAdapter(f, adapterId);
        return f;
    }

    @Override
    public com.zeroc.Ice.LocatorRegistryPrx getRegistry(com.zeroc.Ice.Current current)
    {
        return _registry;
    }

    private final LookupI _lookup;
    private final com.zeroc.Ice.LocatorRegistryPrx _registry;
}
