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

#include <Ice/Ice.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/AdapterI.h>

using namespace std;
using namespace IcePack;

IcePack::LocatorRegistryI::LocatorRegistryI(const AdapterRegistryPtr& adapterRegistry, 
					    const Ice::ObjectAdapterPtr& adapter) :
    _adapterRegistry(adapterRegistry),
    _adapter(adapter)
{
}

void 
IcePack::LocatorRegistryI::setAdapterDirectProxy(const string& id, const Ice::ObjectPrx& proxy, const Ice::Current&)
{
    while(true)
    {
	try
	{
	    //
	    // Get the adapter from the registry and set its direct proxy.
	    //
	    _adapterRegistry->findById(id)->setDirectProxy(proxy);
	    return;
	}
	catch(const AdapterNotExistException&)
	{
	}
	catch(const AdapterActiveException&)
	{
	    throw Ice::AdapterAlreadyActiveException();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    //
	    // Expected if the adapter was destroyed.
	    //
	}
	catch(const Ice::LocalException& ex)
	{
	    //
	    // TODO: We couldn't contact the adapter object. This
	    // is possibly because the IcePack node is down and
	    // the server is started manually for example. We
	    // should probably throw here to prevent the server
	    // from starting?
	    //
	    return;
	}

	//
	// TODO: Review this functionnality.
	//
	// Create a new standalone adapter. This adapter will be
	// destroyed when the registry is shutdown. Since it's not
	// persistent, it won't be restored when the registry startup
	// again. We could change this to make the adapter persistent
	// but then it's not clear when this adapter should be
	// destroyed.
	//
	// Also, is this really usefull? This allows a server (not
	// deployed or managed by an IcePack node) to use the location
	// mechanism (and avoid to share endpoints configuration
	// between the client and server). Maybe it would be less
	// confusing to just prevent servers to start if the server
	// didn't previously registered its object adapters (using the
	// IcePack deployment mechanism).
	//
	Ice::PropertiesPtr properties = _adapter->getCommunicator()->getProperties();
	if(properties->getPropertyAsInt("IcePack.Registry.DynamicRegistration") > 0)
	{
	    AdapterPrx adapter = AdapterPrx::uncheckedCast(_adapter->addWithUUID(new StandaloneAdapterI()));
	    try
	    {
		_adapterRegistry->add(id, adapter);
	    }
	    catch(const AdapterExistsException&)
	    {
		_adapter->remove(adapter->ice_getIdentity());
	    }
	}
	else
	{
	    throw Ice::AdapterNotFoundException();
	}
    }
}

