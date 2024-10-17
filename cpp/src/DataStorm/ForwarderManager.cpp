//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ForwarderManager.h"

using namespace std;
using namespace DataStormI;

ForwarderManager::ForwarderManager(const Ice::ObjectAdapterPtr& adapter, const string& category)
    : _adapter(adapter),
      _category(category),
      _nextId(0)
{
}

Ice::ObjectPrx
ForwarderManager::add(function<void(Ice::ByteSeq, Response, Exception, const Ice::Current&)> forwarder)
{
    lock_guard<mutex> lock(_mutex);
    ostringstream os;
    os << _nextId++;
    _forwarders.emplace(os.str(), std::move(forwarder));
    return _adapter->createProxy({os.str(), _category});
}

Ice::ObjectPrx
ForwarderManager::add(function<void(Ice::ByteSeq, const Ice::Current&)> forwarder)
{
    return add(
        [forwarder = std::move(forwarder)](auto inEncaps, auto response, auto exception, auto current)
        {
            try
            {
                forwarder(inEncaps, current);
                response(true, Ice::ByteSeq());
            }
            catch (...)
            {
                exception(current_exception());
            }
        });
}

void
ForwarderManager::remove(const Ice::Identity& id)
{
    lock_guard<mutex> lock(_mutex);
    _forwarders.erase(id.name);
}

void
ForwarderManager::destroy()
{
    lock_guard<mutex> lock(_mutex);
    _forwarders.clear();
}

void
ForwarderManager::ice_invokeAsync(
    Ice::ByteSeq inEncaps,
    function<void(bool, const Ice::ByteSeq&)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    std::function<void(Ice::ByteSeq, Response, Exception, const Ice::Current&)> forwarder;
    {
        lock_guard<mutex> lock(_mutex);
        auto p = _forwarders.find(current.id.name);
        if (p == _forwarders.end())
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
        }
        forwarder = p->second;
    }
    forwarder(std::move(inEncaps), std::move(response), std::move(exception), current);
}
