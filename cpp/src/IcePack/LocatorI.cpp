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
#include <IcePack/LocatorI.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::LocatorI::LocatorI(const AdapterManagerPrx& adapters, const LocatorRegistryPrx& registry) :
    _adapters(adapters),
    _registry(registry)
{
}

Ice::ObjectPrx 
IcePack::LocatorI::findAdapterByName(const string& name, const Current&) const
{
    try
    {
	AdapterPrx adapter = _adapters->findByName(name);
	if(adapter)
	{
	    return adapter->getDirectProxy(true);
	}
    }
    catch (const AdapterActivationTimeoutException&)
    {
    }
    catch (const ObjectNotExistException&)
    {
    }

    return 0;
}

Ice::LocatorRegistryPrx
IcePack::LocatorI::getRegistry(const Current&) const
{
    return _registry;
}
