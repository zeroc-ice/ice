// **********************************************************************
//
// Copyright (c) 2001
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
    findAdapterByName(String adapter, Ice.Current current)
    {
	return _registry.getAdapter(adapter);
    }
    
    public Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
	return _registryPrx;
    }
    
    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;

}

