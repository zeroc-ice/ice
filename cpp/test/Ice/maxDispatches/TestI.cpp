//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "TestHelper.h"

#include <chrono>
#include <thread>

using namespace std;

class OpTimerTask final : public Ice::TimerTask
{
public:
    OpTimerTask(TestIntfI* servant, function<void()> response) : _servant(servant), _response(std::move(response)) {}

    void runTimerTask() override
    {
        _servant->decDispatchCount();
        _response();
    }

private:
    TestIntfI* _servant;
    std::function<void()> _response;
};

TestIntfI::~TestIntfI() { _timer.destroy(); }

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

    // We use a timer to make this implementation very lightweight.
    _timer.schedule(make_shared<OpTimerTask>(this, std::move(response)), chrono::milliseconds(50));
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

void
TestIntfI::decDispatchCount()
{
    lock_guard<mutex> lock(_mutex);
    --_dispatchCount;
}
