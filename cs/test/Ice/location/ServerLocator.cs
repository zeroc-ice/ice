// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class ServerLocator : Ice.Locator_Disp
{
    public ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
    }
    
    // TODO: These should be AMD operations.

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
