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

#include <Ice/Ice.h>
#include <IcePack/LocatorI.h>

using namespace std;
using namespace IcePack;

IcePack::LocatorI::LocatorI(const AdapterRegistryPtr& adapterRegistry, 
			    const ObjectRegistryPtr& objectRegistry, 
			    const Ice::LocatorRegistryPrx& locatorRegistry) :
    _adapterRegistry(adapterRegistry),
    _objectRegistry(objectRegistry),
    _locatorRegistry(locatorRegistry)
{
}

//
// Find an object by identity. The object is searched in the object
// registry. If found and if the object was registered with an
// adapter, we get the adapter direct proxy and return a proxy created
// from the adapter direct proxy and the object identity. We could
// just return the registered proxy but this would be less efficient
// since the client would have to make a second call to find out the
// adapter direct proxy.
//
Ice::ObjectPrx 
IcePack::LocatorI::findObjectById(const Ice::Identity& id, const Ice::Current& current) const
{
    ObjectDescription obj;

    try
    {
	obj = _objectRegistry->getObjectDescription(id);
    }
    catch(const ObjectNotExistException&)
    {
	throw Ice::ObjectNotFoundException();
    }

    if(!obj.adapterId.empty())
    {
	try
	{
	    Ice::ObjectPrx directProxy = findAdapterById(obj.adapterId, current);
	    if(directProxy)
	    {
		return directProxy->ice_newIdentity(id);
	    }
	}
	catch(Ice::AdapterNotFoundException&)
	{
	    //
	    // Ignore.
	    //
	}
    }

    return obj.proxy;
}
    
Ice::ObjectPrx 
IcePack::LocatorI::findAdapterById(const string& id, const Ice::Current&) const
{
    //
    // TODO: I think will need to do something more sensible in cases
    // where the adapter is found but the adapter proxy is null
    // (possibly because the server activation failed or timed out) or
    // if the adapter object isn't reachable (possibly because the
    // IcePack node is down or unreachable). Right now the Ice client
    // will always throw a NoEndpointException because we return a
    // null proxy here...
    //

    try
    {
	return _adapterRegistry->findById(id)->getDirectProxy(true);
    }
    catch(const AdapterNotExistException&)
    {
	throw Ice::AdapterNotFoundException();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// Expected if the adapter is destroyed.
	//
	throw Ice::AdapterNotFoundException();
    }
    catch(const Ice::NoEndpointException&)
    {
	//
	// This could be because we can't locate the IcePack node
	// adapter (IcePack server adapter proxies are not direct
	// proxies)
	//
    }
    catch(const Ice::LocalException&)
    {
	//
	// Expected if we couldn't contact the adapter object
	// (possibly because the IcePack node is down).
	//
    }

    return 0;
}

Ice::LocatorRegistryPrx
IcePack::LocatorI::getRegistry(const Ice::Current&) const
{
    return _locatorRegistry;
}
