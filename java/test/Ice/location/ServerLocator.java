// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerLocator extends Ice._LocatorDisp
{
    public
    ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
	_registry = registry;
	_registryPrx = registryPrx;
    }

    public void
    findAdapterById_async(Ice.AMD_Locator_findAdapterById response, String adapter, Ice.Current current)
	throws Ice.AdapterNotFoundException
    {
	response.ice_response(_registry.getAdapter(adapter));
    }

    public void
    findObjectById_async(Ice.AMD_Locator_findObjectById response, Ice.Identity id, Ice.Current current)
	throws Ice.ObjectNotFoundException
    {
	response.ice_response(_registry.getObject(id));
    }
    
    public Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
	return _registryPrx;
    }
    
    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;

}

