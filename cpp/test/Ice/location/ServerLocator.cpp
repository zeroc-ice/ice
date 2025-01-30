// Copyright (c) ZeroC, Inc.

#include "ServerLocator.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Ice;

ServerLocatorRegistry::ServerLocatorRegistry() = default;

void
ServerLocatorRegistry::setAdapterDirectProxyAsync(
    string adapter,
    optional<ObjectPrx> object,
    function<void()> response,
    function<void(exception_ptr)>,
    const Current&)
{
    if (!object)
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
ServerLocatorRegistry::setReplicatedAdapterDirectProxyAsync(
    string adapter,
    string replicaGroup,
    optional<ObjectPrx> object,
    function<void()> response,
    function<void(exception_ptr)>,
    const Current&)
{
    if (!object)
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
ServerLocatorRegistry::setServerProcessProxyAsync(
    string,
    optional<ProcessPrx>,
    function<void()> response,
    function<void(exception_ptr)>,
    const Current&)
{
    response();
}

void
ServerLocatorRegistry::addObject(optional<ObjectPrx> object, const Current&)
{
    addObject(object);
}

optional<ObjectPrx>
ServerLocatorRegistry::getAdapter(const string& adapter) const
{
    auto p = _adapters.find(adapter);
    if (_adapters.find(adapter) == _adapters.end())
    {
        throw AdapterNotFoundException();
    }
    return p->second;
}

optional<ObjectPrx>
ServerLocatorRegistry::getObject(const Identity& id) const
{
    auto p = _objects.find(id);
    if (p == _objects.end())
    {
        throw ObjectNotFoundException();
    }

    return p->second;
}

void
ServerLocatorRegistry::addObject(const optional<ObjectPrx>& object)
{
    _objects[object->ice_getIdentity()] = object;
}

ServerLocator::ServerLocator(ServerLocatorRegistryPtr registry, const optional<LocatorRegistryPrx>& registryPrx)
    : _registry(std::move(registry)),
      _registryPrx(registryPrx)
{
}

void
ServerLocator::findObjectByIdAsync(
    Identity id,
    function<void(const optional<ObjectPrx>&)> response,
    function<void(exception_ptr)>,
    const Current&) const
{
    ++const_cast<int&>(_requestCount);
    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    this_thread::sleep_for(chrono::milliseconds(1));
    response(_registry->getObject(id));
}

void
ServerLocator::findAdapterByIdAsync(
    string id,
    function<void(const optional<ObjectPrx>&)> response,
    function<void(exception_ptr)>,
    const Current& current) const
{
    ++const_cast<int&>(_requestCount);
    if (id == "TestAdapter10" || id == "TestAdapter10-2")
    {
        test(current.encoding == Encoding_1_0);
        response(_registry->getAdapter("TestAdapter"));
        return;
    }

    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    this_thread::sleep_for(chrono::milliseconds(1));
    response(_registry->getAdapter(id));
}

optional<LocatorRegistryPrx>
ServerLocator::getRegistry(const Current&) const
{
    return _registryPrx;
}

int
ServerLocator::getRequestCount(const Current&) const
{
    return _requestCount;
}
