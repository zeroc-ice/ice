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
#include <IcePack/AdapterManager.h>
#include <IcePack/LocatorRegistryI.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::LocatorRegistryI::LocatorRegistryI(const AdapterManagerPrx& adapters) :
    _adapters(adapters)
{
}

void 
IcePack::LocatorRegistryI::addAdapter(const string& name, const ObjectPrx& proxy, const Current&)
{
    while(true)
    {
	//
	// Get the adapter from the manager.
	//
	AdapterPrx adapter = _adapters->findByName(name);

	if(adapter)
	{
	    //
	    // Set the adapter direct proxy and return.
	    // 
	    try
	    {
		adapter->setDirectProxy(proxy);

		//
		// The adapter is ready to receive requests even if
		// it's not active.
		//
		adapter->markAsActive();

		return;
	    }
	    catch (const ObjectNotExistException&)
	    {
	    }
	}

	//
	// Adapter doesn't exist yet, create the adapter.
	//
	AdapterDescription desc;
	desc.name = name;
	desc.server = 0;
	    
	try
	{
	    adapter = _adapters->create(desc);
	}
	catch (const AdapterExistsException&)
	{
	    //
	    // Ignore, another thread probably created at the same
	    // time this adapter.
	    //
	}
    }
}
