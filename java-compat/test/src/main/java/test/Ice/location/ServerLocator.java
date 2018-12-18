// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test._TestLocatorDisp;

public class ServerLocator extends _TestLocatorDisp
{
    public
    ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
        _requestCount = 0;
    }

    @Override
    public void
    findAdapterById_async(Ice.AMD_Locator_findAdapterById response, String adapter, Ice.Current current)
        throws Ice.AdapterNotFoundException
    {
        ++_requestCount;
        if(adapter.equals("TestAdapter10") || adapter.equals("TestAdapter10-2"))
        {
            assert(current.encoding.equals(Ice.Util.Encoding_1_0));
            response.ice_response(_registry.getAdapter("TestAdapter"));
            return;
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
        response.ice_response(_registry.getAdapter(adapter));
    }

    @Override
    public void
    findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id, Ice.Current current)
        throws Ice.ObjectNotFoundException
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
        response.ice_response(_registry.getObject(id));
    }

    @Override
    public Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
        return _registryPrx;
    }

    @Override
    public int
    getRequestCount(Ice.Current current)
    {
        return _requestCount;
    }

    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;
    private int _requestCount;

}
