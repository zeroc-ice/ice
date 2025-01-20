// Copyright (c) ZeroC, Inc.

#include "ForwarderManager.h"

using namespace std;
using namespace DataStormI;
using namespace Ice;

ForwarderManager::ForwarderManager(ObjectAdapterPtr adapter, string category)
    : _adapter{std::move(adapter)},
      _category{std::move(category)}
{
}

void
ForwarderManager::remove(const Identity& id)
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
    ByteSeq inParams,
    function<void(bool, const ByteSeq&)> response,
    function<void(exception_ptr)> exception,
    const Current& current)
{
    std::function<void(ByteSeq, Response, Exception, const Current&)> forwarder;
    {
        lock_guard<mutex> lock(_mutex);
        auto p = _forwarders.find(current.id.name);
        if (p == _forwarders.end())
        {
            throw ObjectNotExistException{__FILE__, __LINE__};
        }
        forwarder = p->second;
    }
    forwarder(inParams, response, exception, current);
}
