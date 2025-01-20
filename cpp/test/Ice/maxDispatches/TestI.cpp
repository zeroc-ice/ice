// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "TestHelper.h"

using namespace std;

void
ResponderI::start(const Ice::Current&)
{
    vector<std::function<void()>> responses;
    {
        lock_guard<mutex> lock(_mutex);
        _started = true;
        responses.swap(_responses);
    }

    for (const auto& response : responses)
    {
        response();
    }
}

void
ResponderI::stop(const Ice::Current&)
{
    lock_guard<mutex> lock(_mutex);
    _started = false;
}

int32_t
ResponderI::pendingResponseCount(const Ice::Current&)
{
    lock_guard<mutex> lock(_mutex);
    return static_cast<int32_t>(_responses.size());
}

void
ResponderI::queueResponse(std::function<void()> response)
{
    bool queued = false;
    {
        lock_guard<mutex> lock(_mutex);
        if (!_started)
        {
            _responses.push_back(std::move(response));
            queued = true;
        }
    }

    if (!queued)
    {
        response();
    }
}

void
TestIntfI::opAsync(function<void()> response, function<void(std::exception_ptr)>, const Ice::Current&)
{
    {
        lock_guard<mutex> lock(_mutex);
        if (++_dispatchCount > _maxDispatchCount)
        {
            _maxDispatchCount = _dispatchCount;
        }
    }

    _responder->queueResponse(
        [this, response = std::move(response)]()
        {
            decDispatchCount();
            response();
        });
}

int32_t
TestIntfI::resetMaxConcurrentDispatches(const Ice::Current&)
{
    lock_guard<mutex> lock(_mutex);
    int32_t result = _maxDispatchCount;
    _maxDispatchCount = 0;
    return result;
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

TestIntfI::TestIntfI(shared_ptr<ResponderI> responder) : _responder(std::move(responder)) {}

void
TestIntfI::decDispatchCount()
{
    lock_guard<mutex> lock(_mutex);
    --_dispatchCount;
}
