// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/AdapterFactory.h>

using namespace std;
using namespace IcePack;

IcePack::LocatorRegistryI::LocatorRegistryI(const AdapterRegistryPtr& adapterRegistry, 
                                            const ServerRegistryPtr& serverRegistry,
					    const AdapterFactoryPtr& adapterFactory,
					    bool dynamicRegistration) :
    _adapterRegistry(adapterRegistry),
    _serverRegistry(serverRegistry),
    _adapterFactory(adapterFactory),
    _dynamicRegistration(dynamicRegistration)
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
	catch(const Ice::LocalException&)
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
	// Create a new standalone adapter. The adapter will be
	// persistent. It's the responsability of the user to cleanup
	// adapter entries which were dynamically added from the
	// registry.
	//
	if(_dynamicRegistration)
	{
	    try
	    {
		AdapterPrx adapter = _adapterFactory->createStandaloneAdapter(id);
		try
		{
		    _adapterRegistry->add(id, adapter);
		}
		catch(const AdapterExistsException&)
		{
		    adapter->destroy();
		}
	    }
	    catch(const Ice::AlreadyRegisteredException&)
	    {
	    }
	}
	else
	{
	    throw Ice::AdapterNotFoundException();
	}
    }
}

void
IcePack::LocatorRegistryI::setServerProcessProxy(const string& name, const Ice::ProcessPrx& proxy, const Ice::Current&)
{
    try
    {
        //
        // Get the server from the registry and set its process proxy.
        //
        _serverRegistry->findByName(name)->setProcess(proxy);
        return;
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const Ice::ObjectNotExistException&)
    {
        //
        // Expected if the server was destroyed.
        //
    }
    catch(const Ice::LocalException&)
    {
        //
        // TODO: We couldn't contact the server object. This
        // is possibly because the IcePack node is down and
        // the server is started manually for example. We
        // should probably throw here to prevent the server
        // from starting?
        //
        return;
    }
    throw Ice::ServerNotFoundException();
}
