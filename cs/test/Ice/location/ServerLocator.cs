// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerLocator : Ice.LocatorDisp_
{
    public ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
    }
    
    public override void findAdapterById_async(Ice.AMD_Locator_findAdapterById response, string adapter,
    					       Ice.Current current)
    {
        response.ice_response(_registry.getAdapter(adapter));
    }
    
    public override void findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id,
    					      Ice.Current current)
    {
        response.ice_response(_registry.getObject(id));
    }
    
    public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
    {
        return _registryPrx;
    }
    
    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;
}
