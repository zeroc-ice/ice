// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class ServerLocator : Ice.Locator_Disp
{
    public ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
    }
    
    // TODO: These should be AMD operations. This makes things compile, but prevents the
    // tests from running because it causes deadlock.

    public override Ice.ObjectPrx findAdapterById(string adapter, Ice.Current current)
    {
	return _registry.getAdapter(adapter);
    }

    public override Ice.ObjectPrx findObjectById(Ice.Identity id, Ice.Current current)
    {
	return _registry.getObject(id);
    }

    /*
    public virtual void findAdapterById_async(Ice.AMD_Locator_findAdapterById response, string adapter,
    					      Ice.Current current)
    {
        response.ice_response(_registry.getAdapter(adapter));
    }
    
    public virtual void findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id,
    					     Ice.Current current)
    {
        response.ice_response(_registry.getObject(id));
    }
    */
    
    public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
    {
        return _registryPrx;
    }
    
    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;
}
