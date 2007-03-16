// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifdef ICEE_HAS_LOCATOR
#include <IceE/IceE.h>

#include <IceE/BuiltinSequences.h>
#include <ServerLocator.h>

using namespace std;

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

void 
ServerLocatorRegistry::setAdapterDirectProxy(const ::std::string& adapter, const ::Ice::ObjectPrx& object, 
					     const ::Ice::Current&)
{
    _adapters[adapter] = object;
}

void 
ServerLocatorRegistry::setReplicatedAdapterDirectProxy(const std::string& adapter, const std::string& replicaId, 
						       const ::Ice::ObjectPrx& object, const ::Ice::Current&)
{
    _adapters[adapter] = object;
    _adapters[replicaId] = object;
}

Ice::ObjectPrx
ServerLocatorRegistry::getAdapter(const ::std::string& adapter) const
{
    ::std::map< string, ::Ice::ObjectPrx>::const_iterator p = _adapters.find(adapter);
    if(_adapters.find(adapter) == _adapters.end())
    {
	throw Ice::AdapterNotFoundException();
    }
    return p->second;
}

Ice::ObjectPrx
ServerLocatorRegistry::getObject(const ::Ice::Identity& id) const
{
    ::std::map< ::Ice::Identity, ::Ice::ObjectPrx>::const_iterator p = _objects.find(id);
    if(p == _objects.end())
    {
	throw Ice::ObjectNotFoundException();
    }

    return p->second;
}

void
ServerLocatorRegistry::addObject(const Ice::ObjectPrx& object)
{
    _objects[object->ice_getIdentity()] = object;
}

void
ServerLocatorRegistry::clear()
{
    _objects.clear();
    _adapters.clear();
}

ServerLocator::ServerLocator(const ServerLocatorRegistryPtr& registry, const ::Ice::LocatorRegistryPrx& registryPrx) :
    _registry(registry),
    _registryPrx(registryPrx)
{
}

Ice::ObjectPrx
ServerLocator::findObjectById(const Ice::Identity& id, const Ice::Current& current) const
{
    return _registry->getObject(id);
}

Ice::ObjectPrx
ServerLocator::findAdapterById(const string& id, const Ice::Current& current) const
{
    return _registry->getAdapter(id);
}

Ice::LocatorRegistryPrx
ServerLocator::getRegistry(const ::Ice::Current&) const
{
    return _registryPrx;
}

#endif
