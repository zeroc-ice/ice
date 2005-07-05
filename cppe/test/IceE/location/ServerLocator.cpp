// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifndef ICE_NO_LOCATOR
#include <IceE/IceE.h>

#include <IceE/BuiltinSequences.h>
#include <ServerLocator.h>

using namespace std;

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

void 
ServerLocatorRegistry::setAdapterDirectProxy(const ::std::string& adapter, const ::IceE::ObjectPrx& object, 
					     const ::IceE::Current&)
{
    _adapters[adapter] = object;
}

IceE::ObjectPrx
ServerLocatorRegistry::getAdapter(const ::std::string& adapter) const
{
    ::std::map< string, ::IceE::ObjectPrx>::const_iterator p = _adapters.find(adapter);
    if(_adapters.find(adapter) == _adapters.end())
    {
	throw IceE::AdapterNotFoundException();
    }
    return p->second;
}

IceE::ObjectPrx
ServerLocatorRegistry::getObject(const ::IceE::Identity& id) const
{
    ::std::map< ::IceE::Identity, ::IceE::ObjectPrx>::const_iterator p = _objects.find(id);
    if(p == _objects.end())
    {
	throw IceE::ObjectNotFoundException();
    }

    return p->second;
}

void
ServerLocatorRegistry::addObject(const IceE::ObjectPrx& object)
{
    _objects[object->ice_getIdentity()] = object;
}

ServerLocator::ServerLocator(const ServerLocatorRegistryPtr& registry, const ::IceE::LocatorRegistryPrx& registryPrx) :
    _registry(registry),
    _registryPrx(registryPrx)
{
}

IceE::ObjectPrx
ServerLocator::findObjectById(const IceE::Identity& id, const IceE::Current& current) const
{
    return _registry->getObject(id);
}

IceE::ObjectPrx
ServerLocator::findAdapterById(const string& id, const IceE::Current& current) const
{
    return _registry->getAdapter(id);
}

IceE::LocatorRegistryPrx
ServerLocator::getRegistry(const ::IceE::Current&) const
{
    return _registryPrx;
}

#endif
