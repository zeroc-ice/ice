// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TestI.h>
#include <TestCommon.h>

HoldI::HoldI(const IceUtil::TimerPtr& timer, const Ice::ObjectAdapterPtr& adapter) :
    _last(0), _timer(timer), _adapter(adapter)
{
}

void
HoldI::putOnHold(Ice::Int milliSeconds, const Ice::Current&)
{
    if(milliSeconds < 0)
    {
        _adapter->hold();
    }
    else if(milliSeconds == 0)
    {
        _adapter->hold();
        _adapter->activate();
    }
    else
    {
        try
        {
            _timer->schedule(this, IceUtil::Time::milliSeconds(milliSeconds));
        }
        catch(const IceUtil::IllegalArgumentException&)
        {
        }
    }
}

void
HoldI::waitForHold(const Ice::Current& current)
{
    class WaitForHold : public IceUtil::TimerTask
    {
    public:

        WaitForHold(const Ice::ObjectAdapterPtr& adapter) : _adapter(adapter)
        {
        }

        void 
        runTimerTask()
        {
            try
            {
                _adapter->waitForHold();
                _adapter->activate();
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
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
        _timer->schedule(new WaitForHold(current.adapter), IceUtil::Time());
    }
    catch(const IceUtil::IllegalArgumentException&)
    {
    }
}

Ice::Int
HoldI::set(Ice::Int value, Ice::Int delay, const Ice::Current&)
{
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(delay));

    Lock sync(*this);
    Ice::Int tmp = _last;
    _last = value;
    return tmp;
}

void
HoldI::setOneway(Ice::Int value, Ice::Int expected, const Ice::Current&)
{
    Lock sync(*this);
    test(_last == expected);
    _last = value;
}

void
HoldI::shutdown(const Ice::Current&)
{
    _adapter->hold();
    _adapter->getCommunicator()->shutdown();
}

void
HoldI::runTimerTask()
{
    try
    {
        _adapter->hold();
        _adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
}
