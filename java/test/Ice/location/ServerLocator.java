// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

