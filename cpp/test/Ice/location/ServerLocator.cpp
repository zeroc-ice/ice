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
#include <ServerLocator.h>

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

void 
ServerLocatorRegistry::setAdapterDirectProxy(const ::std::string& adapter, const ::Ice::ObjectPrx& object, 
					     const ::Ice::Current&)
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
ServerLocator::findAdapterById(const ::std::string& adapter, const ::Ice::Current&) const
{
    return _registry->getAdapter(adapter);
}

Ice::LocatorRegistryPrx
ServerLocator::getRegistry(const ::Ice::Current&) const
{
    return _registryPrx;
}

