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

namespace IceGrid
{

template<class TPrx, class FPrx>
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

    SessionKeepAliveThread(const FPrx& factory) : 
	_factory(factory),
	_state(Disconnected),
	_destroySession(false)
    {
    }

    virtual void
    run()
    {
	TPrx session;
	FPrx factory = _factory;
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
		session = createSession(factory, timeout);
		updateState |= session;
	    }		

	    //
	    // If we failed to create the session with the factory and
	    // the factory proxy is a direct proxy, we check with the
	    // Query interface if the factory proxy was updated. It's
	    // possible that the factory was restarted for example.
	    //
	    if(!session && !factory->ice_getEndpoints().empty())
	    {
		std::string instanceName = factory->ice_getIdentity().category;
		try
		{
		    QueryPrx query = QueryPrx::uncheckedCast(
			factory->ice_getCommunicator()->stringToProxy(instanceName + "/Query"));
		    Ice::ObjectPrx obj = query->findObjectById(factory->ice_getIdentity());
		    FPrx newFactory = FPrx::uncheckedCast(obj);
		    if(newFactory != factory)
		    {
			session = createSession(newFactory, timeout);
			factory = newFactory;
			updateState |= session;
		    }
		}
		catch(const Ice::LocalException&)
		{
		}
	    }
	    
	    if(updateState)
	    {
		Lock sync(*this);
		if(_state != Destroyed)
		{
		    _state = session ? Connected : Disconnected;
		}
		_factory = factory;
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
		factory = _factory;
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
    tryCreateSession(FPrx factory)
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
	    if(factory)
	    {
		_factory = factory;
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

    virtual TPrx createSession(const FPrx&, IceUtil::Time&) = 0;
    virtual void destroySession(const TPrx&) = 0;
    virtual bool keepAlive(const TPrx&) = 0;

protected:

    FPrx _factory;
    TPrx _session;
    State _state;
    bool _destroySession;
};

};

#endif
