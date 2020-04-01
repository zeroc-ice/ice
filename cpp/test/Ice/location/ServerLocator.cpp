//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <TestHelper.h>
#include <ServerLocator.h>

using namespace std;

ServerLocatorRegistry::ServerLocatorRegistry()
{
}

void
ServerLocatorRegistry::setAdapterDirectProxyAsync(string adapter, ::shared_ptr<::Ice::ObjectPrx> object,
                                                  function<void()> response,
                                                  function<void(exception_ptr)>,
                                                  const ::Ice::Current&)
{
    if(!object)
    {
        _adapters.erase(adapter);
    }
    else
    {
        _adapters[adapter] = object;
    }
    response();
}

void
ServerLocatorRegistry::setReplicatedAdapterDirectProxyAsync(string adapter, string replicaGroup, shared_ptr<Ice::ObjectPrx> object,
                                                            function<void()> response,
                                                            function<void(exception_ptr)>,
                                                            const ::Ice::Current&)
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
    response();
}

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

void
ServerLocator::findObjectByIdAsync(::Ice::Identity id,
                                   function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                                   function<void(exception_ptr)>,
                                   const ::Ice::Current&) const
{
    ++const_cast<int&>(_requestCount);
    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(1));
    response(_registry->getObject(id));
}

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
