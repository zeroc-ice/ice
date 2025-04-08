// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.AdapterNotFoundException;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.ObjectNotFoundException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Util;

import test.Ice.location.Test.TestLocator;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class ServerLocator implements TestLocator {
    public ServerLocator(
            ServerLocatorRegistry registry, LocatorRegistryPrx registryPrx) {
        _registry = registry;
        _registryPrx = registryPrx;
        _requestCount = 0;
    }

    @Override
    public CompletionStage<ObjectPrx> findAdapterByIdAsync(
            String adapter, Current current)
        throws AdapterNotFoundException {
        ++_requestCount;
        if ("TestAdapter10".equals(adapter) || "TestAdapter10-2".equals(adapter)) {
            assert (current.encoding.equals(Util.Encoding_1_0));
            return CompletableFuture.completedFuture(_registry.getAdapter("TestAdapter"));
        }

        // We add a small delay to make sure locator request queuing gets tested when running the
        // test on a fast machine
        try {
            Thread.sleep(1);
        } catch (InterruptedException ex) {
        }
        return CompletableFuture.completedFuture(_registry.getAdapter(adapter));
    }

    @Override
    public CompletionStage<ObjectPrx> findObjectByIdAsync(
            Identity id, Current current)
        throws ObjectNotFoundException {
        ++_requestCount;
        // We add a small delay to make sure locator request queuing gets tested when running the
        // test on a fast machine
        try {
            Thread.sleep(1);
        } catch (InterruptedException ex) {
        }
        return CompletableFuture.completedFuture(_registry.getObject(id));
    }

    @Override
    public LocatorRegistryPrx getRegistry(Current current) {
        return _registryPrx;
    }

    @Override
    public int getRequestCount(Current current) {
        return _requestCount;
    }

    private final ServerLocatorRegistry _registry;
    private final LocatorRegistryPrx _registryPrx;
    private int _requestCount;
}
