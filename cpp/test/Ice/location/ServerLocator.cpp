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
ServerLocatorRegistry::setAdapterDirectProxy(string adapter, optional<ObjectPrx> object, const Current&)
{
    lock_guard lock(_mutex);
    ++_setRequestCount;
    if (!object)
    {
        _adapters.erase(adapter);
    }
    else
    {
        _adapters[adapter] = object;
    }
}

void
ServerLocatorRegistry::setReplicatedAdapterDirectProxy(
    string adapter,
    string replicaGroup,
    optional<ObjectPrx> object,
    const Current&)
{
    lock_guard lock(_mutex);
    ++_setRequestCount;
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
}

void
ServerLocatorRegistry::setServerProcessProxy(string, optional<ProcessPrx>, const Current&)
{
    lock_guard lock(_mutex);
    ++_setRequestCount;
}

void
ServerLocatorRegistry::addObject(optional<ObjectPrx> object, const Current&)
{
    addObject(object);
}

int32_t
ServerLocatorRegistry::getSetRequestCount(const Current&)
{
    lock_guard lock(_mutex);
    return _setRequestCount;
}

optional<ObjectPrx>
ServerLocatorRegistry::getAdapter(const string& adapter) const
{
    lock_guard lock(_mutex);
    auto p = _adapters.find(adapter);
    if (p == _adapters.end())
    {
        throw AdapterNotFoundException();
    }
    return p->second;
}

optional<ObjectPrx>
ServerLocatorRegistry::getObject(const Identity& id) const
{
    lock_guard lock(_mutex);
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
    lock_guard lock(_mutex);
    _objects[object->ice_getIdentity()] = object;
}

ServerLocator::ServerLocator(ServerLocatorRegistryPtr registry, const optional<LocatorRegistryPrx>& registryPrx)
    : _registry(std::move(registry)),
      _registryPrx(registryPrx)
{
}

optional<ObjectPrx>
ServerLocator::findObjectById(Identity id, const Current&) const
{
    ++_requestCount;
    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    this_thread::sleep_for(chrono::milliseconds(1));
    return _registry->getObject(id);
}

std::optional<ObjectPrx>
ServerLocator::findAdapterById(string id, const Current& current) const
{
    ++_requestCount;
    if (id == "TestAdapter10" || id == "TestAdapter10-2")
    {
        test(current.encoding == Encoding_1_0);
        return _registry->getAdapter("TestAdapter");
    }

    // We add a small delay to make sure locator request queuing gets tested when
    // running the test on a fast machine
    this_thread::sleep_for(chrono::milliseconds(1));
    return _registry->getAdapter(id);
}

optional<LocatorRegistryPrx>
ServerLocator::getRegistry(const Current&) const
{
    return _registryPrx;
}

std::int32_t
ServerLocator::getRequestCount(const Current&) const
{
    return _requestCount;
}
