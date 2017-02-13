// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.ObjectPrx;

import test.Ice.location.Test.TestLocator;

public class ServerLocator implements TestLocator
{
    public ServerLocator(ServerLocatorRegistry registry, com.zeroc.Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
        _requestCount = 0;
    }

    @Override
    public CompletionStage<ObjectPrx> findAdapterByIdAsync(String adapter, com.zeroc.Ice.Current current)
        throws com.zeroc.Ice.AdapterNotFoundException
    {
        ++_requestCount;
        if(adapter.equals("TestAdapter10") || adapter.equals("TestAdapter10-2"))
        {
            assert(current.encoding.equals(com.zeroc.Ice.Util.Encoding_1_0));
            return CompletableFuture.completedFuture(_registry.getAdapter("TestAdapter"));
        }

        // We add a small delay to make sure locator request queuing gets tested when
        // running the test on a fast machine
        try
        {
            Thread.sleep(1);
        }
        catch(java.lang.InterruptedException ex)
        {
        }
        return CompletableFuture.completedFuture(_registry.getAdapter(adapter));
    }

    @Override
    public CompletionStage<ObjectPrx> findObjectByIdAsync(com.zeroc.Ice.Identity id, com.zeroc.Ice.Current current)
        throws com.zeroc.Ice.ObjectNotFoundException
    {
        ++_requestCount;
        // We add a small delay to make sure locator request queuing gets tested when
        // running the test on a fast machine
        try
        {
            Thread.sleep(1);
        }
        catch(java.lang.InterruptedException ex)
        {
        }
        return CompletableFuture.completedFuture(_registry.getObject(id));
    }

    @Override
    public com.zeroc.Ice.LocatorRegistryPrx getRegistry(com.zeroc.Ice.Current current)
    {
        return _registryPrx;
    }

    @Override
    public int getRequestCount(com.zeroc.Ice.Current current)
    {
        return _requestCount;
    }

    private ServerLocatorRegistry _registry;
    private com.zeroc.Ice.LocatorRegistryPrx _registryPrx;
    private int _requestCount;
}
