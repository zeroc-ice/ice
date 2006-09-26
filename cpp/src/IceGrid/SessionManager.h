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

#include <IceGrid/Query.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

template<class TPrx>
class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
    enum State
    {
	Disconnected,
	Connected,
	Retry,
	DestroySession,
	Destroyed
    };

public:

    SessionKeepAliveThread(const InternalRegistryPrx& registry) : 
	_registry(registry),
	_state(Disconnected),
	_destroySession(false)
    {
    }

    virtual void
    run()
    {
	TPrx session;
	InternalRegistryPrx registry = _registry;
	bool updateState = false;
	IceUtil::Time timeout = IceUtil::Time::seconds(10); 
	bool destroy = false;

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
		session = createSession(registry, timeout);
		updateState |= session;
	    }

	    if(updateState)
	    {
		Lock sync(*this);
		if(_state != Destroyed)
		{
		    _state = session ? Connected : Disconnected;
		}
		_session = session;
		notifyAll();
	    }

	    //
	    // Wait for the configured timeout duration.
	    //
	    {
		Lock sync(*this);
		if(_state == Destroyed)
		{
		    break;
		}
		else if(_state == Connected || _state == Disconnected)
		{
		    timedWait(timeout);
		}
		
		if(_state == Destroyed)
		{
		    break;
		}
		else if(_state == DestroySession && session)
		{
		    destroy = true;
		}

		updateState = _state == Retry || _state == DestroySession;
		registry = _registry;
	    }

	    if(destroy)
	    {
		assert(session);
		destroySession(session);
		destroy = false;
		session = 0;
	    }
	}
	
	//
	// Destroy the session.
	//
	if(_destroySession && session)
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
    tryCreateSession(InternalRegistryPrx registry)
    {
	{
	    Lock sync(*this);
	    while(_state == Retry)
	    {
		wait();
	    }

	    if(_state == Destroyed)
	    {
		return false;
	    }

	    _state = Retry;
	    if(registry)
	    {
		_registry = registry;
	    }
	    notifyAll();
	}
	{
	    Lock sync(*this);
	    while(_state == Retry)
	    {
		wait();
	    }
	}	
	return true;
    }

    void
    destroyActiveSession()
    {
	Lock sync(*this);
	_state = DestroySession;
	notifyAll();	
    }

    void
    terminate(bool destroySession = true)
    {
	Lock sync(*this);
	assert(_state != Destroyed);
	_state = Destroyed;
	_destroySession = destroySession;
	notifyAll();
    }

    TPrx
    getSession()
    {
	Lock sync(*this);
	return _session;
    }

    void
    setRegistry(const InternalRegistryPrx& registry)
    {
	Lock sync(*this);
	_registry = registry;
    }

    InternalRegistryPrx
    getRegistry() const
    {
	Lock sync(*this);
	return _registry;
    }
    
    virtual TPrx createSession(const InternalRegistryPrx&, IceUtil::Time&) = 0;
    virtual void destroySession(const TPrx&) = 0;
    virtual bool keepAlive(const TPrx&) = 0;

protected:

    InternalRegistryPrx _registry;
    TPrx _session;
    State _state;
    bool _destroySession;
};

};

#endif
