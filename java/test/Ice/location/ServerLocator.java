// **********************************************************************
//
// Copyright (c) 2002
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

