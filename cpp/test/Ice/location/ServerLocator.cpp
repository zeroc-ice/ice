// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <ServerLocator.h>

using namespace std;

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

void 
ServerLocatorRegistry::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
                                                   const std::string& adapter, const ::Ice::ObjectPrx& object, 
                                                   const ::Ice::Current&)
{
    _adapters[adapter] = object;
    cb->ice_response();
}

void 
ServerLocatorRegistry::setReplicatedAdapterDirectProxy_async(
    const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const std::string& adapter, const ::std::string& replicaGroup, const ::Ice::ObjectPrx& object, 
    const ::Ice::Current&)
{
    _adapters[adapter] = object;
    _adapters[replicaGroup] = object;
    cb->ice_response();
}

void 
ServerLocatorRegistry::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                                   const ::std::string&, const Ice::ProcessPrx&, const ::Ice::Current&)
{
    cb->ice_response();
}

void
ServerLocatorRegistry::addObject(const Ice::ObjectPrx& object, const ::Ice::Current&)
{
    addObject(object);
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

ServerLocator::ServerLocator(const ServerLocatorRegistryPtr& registry, const ::Ice::LocatorRegistryPrx& registryPrx) :
    _registry(registry),
    _registryPrx(registryPrx),
    _requestCount(0)
{
}

void
ServerLocator::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& response, const Ice::Identity& id, 
                                    const Ice::Current& current) const
{
    ++const_cast<int&>(_requestCount);
    response->ice_response(_registry->getObject(id));
}

void
ServerLocator::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& response, const string& id, 
                                     const Ice::Current& current) const
{
    ++const_cast<int&>(_requestCount);
    response->ice_response(_registry->getAdapter(id));
}

Ice::LocatorRegistryPrx
ServerLocator::getRegistry(const ::Ice::Current&) const
{
    return _registryPrx;
}

int
ServerLocator::getRequestCount(const ::Ice::Current&) const
{
    return _requestCount;
}
