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
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/AdapterI.h>

using namespace std;
using namespace IcePack;

IcePack::LocatorRegistryI::LocatorRegistryI(const AdapterRegistryPtr& registry, const Ice::ObjectAdapterPtr& adapter) :
    _registry(registry),
    _adapter(adapter)
{
}

void 
IcePack::LocatorRegistryI::addAdapter(const string& name, const Ice::ObjectPrx& proxy, const Ice::Current&)
{
    while(true)
    {
	try
	{
	    //
	    // Get the adapter from the registry and set its direct proxy.
	    //
	    _registry->findByName(name)->setDirectProxy(proxy);
	    return;
	}
	catch(const AdapterNotExistException&)
	{
	}
	catch(const AdapterActiveException&)
	{
	    //
	    // TODO: we have to do something here.  We can't just let
	    // the server try to override the direct proxy of an
	    // active adapter without saying anything. We need to
	    // throw here to prevent the server from starting.
	    //
	    return;
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
	    // from starting.
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
	AdapterPrx adapter = AdapterPrx::uncheckedCast(_adapter->addWithUUID(new StandaloneAdapterI()));
	try
	{
	    _registry->add(name, adapter);
	}
	catch(const AdapterExistsException&)
	{
	    _adapter->remove(adapter->ice_getIdentity());
	}
    }
}
