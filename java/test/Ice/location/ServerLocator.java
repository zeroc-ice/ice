// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerLocator extends Test._TestLocatorDisp
{
    public
    ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
        _requestCount = 0;
    }

    public void
    findAdapterById_async(Ice.AMD_Locator_findAdapterById response, String adapter, Ice.Current current)
        throws Ice.AdapterNotFoundException
    {
        ++_requestCount;
        response.ice_response(_registry.getAdapter(adapter));
    }

    public void
    findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id, Ice.Current current)
        throws Ice.ObjectNotFoundException
    {
        ++_requestCount;
        response.ice_response(_registry.getObject(id));
    }
    
    public Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
        return _registryPrx;
    }

    public int
    getRequestCount(Ice.Current current)
    {
        return _requestCount;
    }
    
    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;
    private int _requestCount;

}

