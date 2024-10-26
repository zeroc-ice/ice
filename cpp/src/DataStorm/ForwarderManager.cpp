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
    Ice::ByteSeq inParams,
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
            throw Ice::ObjectNotExistException{__FILE__, __LINE__};
        }
        forwarder = p->second;
    }
    forwarder(std::move(inParams), std::move(response), std::move(exception), current);
}
