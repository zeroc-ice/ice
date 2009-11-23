// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <StateChangerI.h>
#include <TestCommon.h>
#include <functional>
#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

StateChangerI::StateChangerI(const IceUtil::TimerPtr& timer, const Ice::ObjectAdapterPtr& adapter)
    : _action(Hold), _timer(timer), _otherAdapter(adapter)
{
}

void
StateChangerI::hold(Ice::Int milliSeconds, const Ice::Current& current)
{
    if(milliSeconds <= 0)
    {
	_otherAdapter->hold();
	_otherAdapter->waitForHold();
    }
    else
    {
	_action = Hold;
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
StateChangerI::activate(Ice::Int milliSeconds, const Ice::Current&)
{
    if(milliSeconds <= 0)
    {
	_otherAdapter->activate();
    }
    else
    {
	_action = Activate;
	try
	{
	    _timer->schedule(this, IceUtil::Time::milliSeconds(milliSeconds));
	}
	catch(const IceUtil::IllegalArgumentException&)
	{
	}
    }
}

void StateChangerI::runTimerTask()
{
    if(_action == Hold)
    {
        _otherAdapter->hold();
    }
    else
    {
        _otherAdapter->activate();
    }
}
