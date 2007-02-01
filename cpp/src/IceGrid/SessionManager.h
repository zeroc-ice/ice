// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        InProgress,
        Destroyed
    };

    enum Action
    {
        Connect,
        Disconnect,
        KeepAlive,
        None
    };

public:

    SessionKeepAliveThread(const InternalRegistryPrx& registry) : 
        _registry(registry),
        _state(InProgress),
        _nextAction(None)
    {
    }

    virtual void
    run()
    {
        TPrx session;
        InternalRegistryPrx registry;
        IceUtil::Time timeout = IceUtil::Time::seconds(15); 
        Action action = Connect;

        while(true)
        {
            {
                Lock sync(*this);
                if(_state == Destroyed)
                {
                    break;
                }

                //
                // Update the current state.
                //
                assert(_state == InProgress);
                _state = session ? Connected : Disconnected;
                _session = session;
                if(_session)
                {
                    _registry = registry;
                }

                if(_nextAction == Connect && _state == Connected)
                {
                    _nextAction = KeepAlive;
                }
                else if(_nextAction == Disconnect && _state == Disconnected)
                {
                    _nextAction = None;
                }
                else if(_nextAction == KeepAlive && _state == Disconnected)
                {
                    _nextAction = Connect;
                }
                notifyAll();

                //
                // Wait if there's nothing to do and if we are
                // connected or if we've just tried to connect.
                //
                if(_nextAction == None)
                {
                    if(_state == Connected || action == Connect || action == KeepAlive)
                    {
                        IceUtil::Time wakeTime = IceUtil::Time::now() + timeout;
                        while(_state != Destroyed && _nextAction == None)
                        {
                            if(!timedWait(wakeTime - IceUtil::Time::now()))
                            {
                                break;
                            }
                        }
                    }
                    if(_nextAction == None)
                    {
                        _nextAction = session ? KeepAlive : Connect;
                    }
                }

                if(_state == Destroyed)
                {
                    break;
                }
                
                assert(_nextAction != None);
                
                action = _nextAction;
                registry = InternalRegistryPrx::uncheckedCast(
                    _registry->ice_timeout(static_cast<int>(timeout.toMilliSeconds())));
                _nextAction = None;
                _state = InProgress;
                notifyAll();
            }

            switch(action)
            {
            case Connect:
                assert(!session);
                session = createSession(registry, timeout);
                break;
            case Disconnect:
                assert(session);
                destroySession(session);
                session = 0;
                break;
            case KeepAlive:
                assert(session);
                if(!keepAlive(session))
                {
                    session = createSession(registry, timeout);
                }
                break;
            case None:
            default:
                assert(false);
            }
        }
        
        //
        // Destroy the session.
        //
        if(_nextAction == Disconnect && session)
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

    virtual void
    tryCreateSession(bool waitForTry = true)
    {
        {
            Lock sync(*this);
            if(_state == Destroyed)
            {
                return;
            }

            if(_state == Connected)
            {
                _nextAction = KeepAlive;
            }
            else
            {
                _nextAction = Connect;
            }
            notifyAll();
        }

        if(waitForTry)
        {
            Lock sync(*this);
            // Wait until the action is executed and the state changes.
            while(_nextAction == Connect || _nextAction == KeepAlive || _state == InProgress)
            {
                wait();
            }
        }
    }

    void
    destroyActiveSession()
    {
        Lock sync(*this);
        if(_state == Destroyed || _state == Disconnected)
        {
            return;
        }
        _nextAction = Disconnect;
        notifyAll();
    }

    bool
    terminateIfDisconnected()
    {
        Lock sync(*this);
        if(_state != Disconnected)
        {
            return false; // Nothing we can do for now.
        }
        assert(_state != Destroyed);
        _state = Destroyed;
        _nextAction = None;
        notifyAll();
        return true;
    }

    void
    terminate(bool destroySession = true)
    {
        Lock sync(*this);
        if(_state == Destroyed)
        {
            return;
        }
        assert(_state != Destroyed);
        _state = Destroyed;
        _nextAction = destroySession ? Disconnect : None;
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
    
    virtual TPrx createSession(InternalRegistryPrx&, IceUtil::Time&) = 0;
    virtual void destroySession(const TPrx&) = 0;
    virtual bool keepAlive(const TPrx&) = 0;

protected:

    InternalRegistryPrx _registry;
    TPrx _session;
    State _state;
    Action _nextAction;
};

};

#endif
