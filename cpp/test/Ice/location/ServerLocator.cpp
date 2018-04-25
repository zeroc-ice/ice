// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <TestCommon.h>
#include <ServerLocator.h>

using namespace std;

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

#ifdef ICE_CPP11_MAPPING
void
ServerLocatorRegistry::setAdapterDirectProxyAsync(string adapter, ::shared_ptr<::Ice::ObjectPrx> object,
                                                  function<void()> response,
                                                  function<void(exception_ptr)>,
                                                  const ::Ice::Current&)
#else
void
ServerLocatorRegistry::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
                                                   const std::string& adapter, const ::Ice::ObjectPrx& object,
                                                   const ::Ice::Current&)
#endif
{
    if(!object)
    {
        _adapters.erase(adapter);
    }
    else
    {
        _adapters[adapter] = object;
    }
#ifdef ICE_CPP11_MAPPING
    response();
#else
    cb->ice_response();
#endif
}

#ifdef ICE_CPP11_MAPPING
void
ServerLocatorRegistry::setReplicatedAdapterDirectProxyAsync(string adapter, string replicaGroup, shared_ptr<Ice::ObjectPrx> object,
                                                            function<void()> response,
                                                            function<void(exception_ptr)>,
                                                            const ::Ice::Current&)
#else
void
ServerLocatorRegistry::setReplicatedAdapterDirectProxy_async(
    const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const string& adapter, const string& replicaGroup, const ::Ice::ObjectPrx& object,
    const ::Ice::Current&)
#endif
{
    if(!object)
    {
        _adapters.erase(adapter);
        _adapters.erase(replicaGroup);
    }
    else
    {
        _adapters[adapter] = object;
        _adapters[replicaGroup] = object;
    }
#ifdef ICE_CPP11_MAPPING
    response();
#else
    cb->ice_response();
#endif
}

#ifdef ICE_CPP11_MAPPING
void
ServerLocatorRegistry::setServerProcessProxyAsync(string,
                                                  shared_ptr<Ice::ProcessPrx>,
                                                  function<void()> response,
                                                  function<void(exception_ptr)>,
                                                  const ::Ice::Current&)
{
    response();
}

void
ServerLocatorRegistry::addObject(shared_ptr<Ice::ObjectPrx> object, const ::Ice::Current&)
{
    addObject(object);
}
#else
void
ServerLocatorRegistry::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                                   const string&, const Ice::ProcessPrx&, const ::Ice::Current&)
{
    cb->ice_response();
}

void
ServerLocatorRegistry::addObject(const Ice::ObjectPrx& object, const ::Ice::Current&)
{
    addObject(object);
}
#endif

Ice::ObjectPrxPtr
ServerLocatorRegistry::getAdapter(const string& adapter) const
{
    map< string, ::Ice::ObjectPrxPtr>::const_iterator p = _adapters.find(adapter);
    if(_adapters.find(adapter) == _adapters.end())
    {
        throw Ice::AdapterNotFoundException();
    }
    return p->second;
}

Ice::ObjectPrxPtr
ServerLocatorRegistry::getObject(const ::Ice::Identity& id) const
{
    map< ::Ice::Identity, ::Ice::ObjectPrxPtr>::const_iterator p = _objects.find(id);
    if(p == _objects.end())
    {
        throw Ice::ObjectNotFoundException();
    }

    return p->second;
}

void
ServerLocatorRegistry::addObject(const Ice::ObjectPrxPtr& object)
{
    _objects[object->ice_getIdentity()] = object;
}

ServerLocator::ServerLocator(const ServerLocatorRegistryPtr& registry, const ::Ice::LocatorRegistryPrxPtr& registryPrx) :
    _registry(registry),
    _registryPrx(registryPrx),
    _requestCount(0)
{
}

#ifdef ICE_CPP11_MAPPING
void
ServerLocator::findObjectByIdAsync(::Ice::Identity id,
                                   function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                                   function<void(exception_ptr)>,
                                   const ::Ice::Current&) const
#else
void
ServerLocator::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& response, const Ice::Identity& id,
                                    const Ice::Current&) const
#endif
{
    ++const_cast<int&>(_requestCount);
    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
#ifdef ICE_CPP11_MAPPING
    response(_registry->getObject(id));
#else
    response->ice_response(_registry->getObject(id));
#endif
}

#ifdef ICE_CPP11_MAPPING
void
ServerLocator::findAdapterByIdAsync(string id,
                                    function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                                    function<void(exception_ptr)>,
                                    const ::Ice::Current& current) const
{
    ++const_cast<int&>(_requestCount);
    if(id == "TestAdapter10" || id == "TestAdapter10-2")
    {
        test(current.encoding == Ice::Encoding_1_0);
        response(_registry->getAdapter("TestAdapter"));
        return;
    }

    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
    response(_registry->getAdapter(id));
}
#else
void
ServerLocator::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& response, const string& id,
                                     const Ice::Current& current) const
{
    ++const_cast<int&>(_requestCount);
    if(id == "TestAdapter10" || id == "TestAdapter10-2")
    {
        test(current.encoding == Ice::Encoding_1_0);
        response->ice_response(_registry->getAdapter("TestAdapter"));
        return;
    }

    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
    response->ice_response(_registry->getAdapter(id));
}
#endif

Ice::LocatorRegistryPrxPtr
ServerLocator::getRegistry(const ::Ice::Current&) const
{
    return _registryPrx;
}

int
ServerLocator::getRequestCount(const ::Ice::Current&) const
{
    return _requestCount;
}
