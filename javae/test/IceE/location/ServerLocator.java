// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    public Ice.ObjectPrx
    findAdapterById(String adapter, Ice.Current current)
	throws Ice.AdapterNotFoundException
    {
	return _registry.getAdapter(adapter);
    }

    public Ice.ObjectPrx
    findObjectById(Ice.Identity id, Ice.Current current)
	throws Ice.ObjectNotFoundException
    {
	return _registry.getObject(id);
    }
    
    public Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
	return _registryPrx;
    }
    
    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;

}

