//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "IceUtil/IceUtil.h"
#include <TestI.h>
#include <TestHelper.h>

#include <thread>
#include <chrono>
#include <stdexcept>

using namespace std;

HoldI::HoldI(const IceUtil::TimerPtr& timer, const Ice::ObjectAdapterPtr& adapter)
    : _last(0),
      _timer(timer),
      _adapter(adapter)
{
}

void
HoldI::putOnHold(int32_t milliSeconds, const Ice::Current&)
{
    class PutOnHold : public IceUtil::TimerTask
    {
    public:
        PutOnHold(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter) {}

        void runTimerTask()
        {
            try
            {
                _adapter->hold();
                _adapter->activate();
            }
            catch (const Ice::ObjectAdapterDeactivatedException&)
            {
                //
                // This shouldn't occur. The test ensures all the waitForHold timers are
                // finished before shutting down the communicator.
                //
                test(false);
            }
        }

    private:
        const Ice::ObjectAdapterPtr _adapter;
    };

    if (milliSeconds < 0)
    {
        _adapter->hold();
    }
    else if (milliSeconds == 0)
    {
        _adapter->hold();
        _adapter->activate();
    }
    else
    {
        try
        {
            _timer->schedule(make_shared<PutOnHold>(_adapter), chrono::milliseconds(milliSeconds));
        }
        catch (const invalid_argument&)
        {
        }
    }
}

void
HoldI::waitForHold(const Ice::Current& current)
{
    class WaitForHold final : public IceUtil::TimerTask
    {
    public:
        WaitForHold(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter) {}

        void runTimerTask() final
        {
            try
            {
                _adapter->waitForHold();
                _adapter->activate();
            }
            catch (const Ice::ObjectAdapterDeactivatedException&)
            {
                //
                // This shouldn't occur. The test ensures all the waitForHold timers are
                // finished before shutting down the communicator.
                //
                test(false);
            }
        }

    private:
        const Ice::ObjectAdapterPtr _adapter;
    };

    try
    {
        _timer->schedule(make_shared<WaitForHold>(current.adapter), chrono::seconds::zero());
    }
    catch (const invalid_argument&)
    {
    }
}

int32_t
HoldI::set(int32_t value, int32_t delay, const Ice::Current&)
{
    this_thread::sleep_for(chrono::milliseconds(delay));

    lock_guard lock(_mutex);
    int32_t tmp = _last;
    _last = value;
    return tmp;
}

void
HoldI::setOneway(int32_t value, int32_t expected, const Ice::Current&)
{
    lock_guard lock(_mutex);
    test(_last == expected);
    _last = value;
}

void
HoldI::shutdown(const Ice::Current&)
{
    _adapter->hold();
    _adapter->getCommunicator()->shutdown();
}
