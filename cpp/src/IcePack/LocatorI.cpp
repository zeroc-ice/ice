// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/LocatorI.h>

using namespace std;
using namespace IcePack;

IcePack::LocatorI::LocatorI(const AdapterRegistryPtr& adapterRegistry, 
			    const Ice::LocatorRegistryPrx& locatorRegistry) :
    _adapterRegistry(adapterRegistry),
    _locatorRegistry(locatorRegistry)
{
}

Ice::ObjectPrx 
IcePack::LocatorI::findAdapterByName(const string& name, const Ice::Current&) const
{
    //
    // TODO: I think will need to do something more sensible in cases
    // where the adapter is found but the adapter proxy is null
    // (possibly because the server activation failed or timed out) or
    // if the adapter object isn't reachable (possibly because the
    // IcePack node is down or unreachable). Right now the Ice cleint
    // will always throw a NoEndpointException because we return a
    // null proxy here...
    //

    try
    {
	return _adapterRegistry->findByName(name)->getDirectProxy(true);
    }
    catch(const AdapterNotExistException&)
    {
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// Expected if the adapter is destroyed.
	//
    }
    catch(const Ice::NoEndpointException&)
    {
	//
	// This could be because we can't locate the IcePack node
	// adapter. IcePack server adapter proxy are not direct proxy.
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
