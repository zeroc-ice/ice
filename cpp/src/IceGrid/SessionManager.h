// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SESSION_MANAGER_H
#define ICE_GRID_SESSION_MANAGER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h>

namespace IceGrid
{

template<class TPrx, class FPrx>
class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
    enum State
    {
	Initial,
	Disconnected,
	Connected,
	Destroyed
    };

public:

    SessionKeepAliveThread(const FPrx& factory) : 
	_factory(factory),
	_state(Initial)
    {
    }

    virtual void
    run()
    {
	TPrx session;
	FPrx factory = _factory;
	bool updateState = true;
	IceUtil::Time timeout = IceUtil::Time::seconds(10); 

	while(true)
	{
	    //
	    // Send a keep alive message to the session.
	    //
	    if(session)
	    {
		if(!keepAlive(session))
		{
		    session = 0;
		}
		updateState |= !session;
	    }
	    
	    //
	    // If the session isn't established yet, try to create a new
	    // session.
	    //
	    if(!session)
	    {
		session = createSession(factory, timeout);
		updateState |= session;
	    }
	    
	    if(updateState)
	    {
		Lock sync(*this);
		_state = session ? Connected : Disconnected;
		_session = session;
		notifyAll();
	    }

	    //
	    // Wait for the configured timeout duration.
	    //
	    {
		Lock sync(*this);
		
		if(_state != Destroyed)
		{
		    timedWait(timeout);
		}
		
		if(_state == Destroyed)
		{
		    break;
		}
		
		updateState = _state == Initial;
		factory = _factory;
	    }
	}
	
	//
	// Destroy the session.
	//
	if(session)
	{
	    destroySession(session);
	}
    }

    virtual bool
    waitForCreate()
    {
	Lock sync(*this);
	while(_state != Destroyed && _state != Connected)
	{
	    wait();
	}
	return _state != Destroyed;
    }

    virtual bool
    tryCreateSession(FPrx factory)
    {
	FPrx previous;
	{
	    Lock sync(*this);
	    if(_state == Destroyed)
	    {
		return false;
	    }

	    _state = Initial;
	    if(factory)
	    {
		previous = _factory;
		_factory = factory;
	    }
	    notifyAll();
	}
	{
	    Lock sync(*this);
	    while(_state == Initial)
	    {
		wait();
	    }
	    if(factory)
	    {
		_factory = previous; // Restore the previous factory
	    }
	}	
	return true;
    }

    void
    terminate()
    {
	Lock sync(*this);
	_state = Destroyed;
	notifyAll();
    }

    TPrx
    getSession()
    {
	Lock sync(*this);
	return _session;
    }

    virtual TPrx createSession(const FPrx&, IceUtil::Time&) const = 0;
    virtual void destroySession(const TPrx&) const = 0;
    virtual bool keepAlive(const TPrx&) const = 0;

private:

    FPrx _factory;
    TPrx _session;
    State _state;
};

};

#endif
