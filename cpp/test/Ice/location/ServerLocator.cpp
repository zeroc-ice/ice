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
#include <ServerLocator.h>

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

void 
ServerLocatorRegistry::addAdapter(const ::std::string& adapter, const ::Ice::ObjectPrx& object, const ::Ice::Current&)
{
    _adapters[adapter] = object;
}


Ice::ObjectPrx
ServerLocatorRegistry::getAdapter(const ::std::string& adapter)
{
    return _adapters[adapter];
}

ServerLocator::ServerLocator(const ServerLocatorRegistryPtr& registry, const ::Ice::LocatorRegistryPrx& registryPrx) :
    _registry(registry),
    _registryPrx(registryPrx)
{
}

Ice::ObjectPrx
ServerLocator::findAdapterByName(const ::std::string& adapter, const ::Ice::Current&) const
{
    return _registry->getAdapter(adapter);
//    std::map< std::string, Ice::ObjectPrx >::iterator p = _adapters.find(adapter);
//    if(p != _adapters.end())
//    {
//	return p->second;
//    }
//    return 0;
}

Ice::LocatorRegistryPrx
ServerLocator::getRegistry(const ::Ice::Current&) const
{
    return _registryPrx;
}

