// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>

#include <IceGrid/Registry.h>
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

    SessionKeepAliveThread(const InternalRegistryPrx& registry, const Ice::LoggerPtr& logger) :
        IceUtil::Thread("IceGrid session keepalive thread"),
        _registry(registry),
        _logger(logger),
        _state(InProgress),
        _nextAction(None)
    {
    }

    virtual void
    run()
    {
        TPrx session;
        InternalRegistryPrx registry;
        IceUtil::Time timeout = IceUtil::Time::seconds(10);
        Action action = Connect;

        try
        {
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
                            IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
                            IceUtil::Time wakeTime = now + timeout;
                            while(_state != Destroyed && _nextAction == None && wakeTime > now)
                            {
                                timedWait(wakeTime - now);
                                now = IceUtil::Time::now(IceUtil::Time::Monotonic);
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
        catch(const std::exception& ex)
        {
            Ice::Error out(_logger);
            out << "unknown exception in session manager keep alive thread:\n" << ex.what();
            throw;
        }
        catch(...)
        {
            Ice::Error out(_logger);
            out << "unknown exception in session manager keep alive thread";
            throw;
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

    void
    tryCreateSession()
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
    }

    void
    waitTryCreateSession(const IceUtil::Time& timeout = IceUtil::Time())
    {
        Lock sync(*this);
        // Wait until the action is executed and the state changes.
        while(_nextAction == Connect || _nextAction == KeepAlive || _state == InProgress)
        {
            if(timeout == IceUtil::Time())
            {
                wait();
            }
            else
            {
                if(!timedWait(timeout))
                {
                    break;
                }
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

    bool
    isDestroyed()
    {
        Lock sync(*this);
        return _state == Destroyed;
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
    Ice::LoggerPtr _logger;
    TPrx _session;
    State _state;
    Action _nextAction;
};

class SessionManager : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionManager(const Ice::CommunicatorPtr&, const std::string&);
    virtual ~SessionManager();

    virtual bool isDestroyed() = 0;

protected:

    std::vector<IceGrid::QueryPrx> findAllQueryObjects(bool);

    Ice::CommunicatorPtr _communicator;
    std::string _instanceName;
    InternalRegistryPrx _master;
    std::vector<IceGrid::QueryPrx> _queryObjects;
};

};

#endif
