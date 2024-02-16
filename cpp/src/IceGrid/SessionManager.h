//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_SESSION_MANAGER_H
#define ICE_GRID_SESSION_MANAGER_H

#include <Ice/Logger.h>
#include <Ice/LoggerUtil.h>

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>
#include <IceGrid/Util.h>

namespace IceGrid
{

template<class TPrx>
class SessionKeepAliveThread
{
#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wshadow"
#endif
    enum State
    {
        Disconnected,
        Connected,
        InProgress,
        Destroyed
    };
#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

    enum Action
    {
        Connect,
        Disconnect,
        KeepAlive,
        None
    };

public:

    SessionKeepAliveThread(const InternalRegistryPrxPtr& registry,
                           const std::shared_ptr<Ice::Logger>& logger) :
        _registry(registry),
        _logger(logger),
        _state(InProgress),
        _nextAction(None),
        _thread([this] { run(); })
    {
    }

    virtual ~SessionKeepAliveThread()
    {
        assert(_state == Destroyed);
    }

    void
    run()
    {
        using namespace std::chrono_literals;
        std::optional<TPrx> session;
        InternalRegistryPrxPtr registry;
        std::chrono::seconds timeout = 10s;
        Action action = Connect;

        try
        {
            while(true)
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
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
                    _condVar.notify_all();

                    //
                    // Wait if there's nothing to do and if we are
                    // connected or if we've just tried to connect.
                    //
                    if(_nextAction == None)
                    {
                        if(_state == Connected || action == Connect || action == KeepAlive)
                        {
                            auto now = std::chrono::steady_clock::now();
                            auto wakeTime = now + timeout;
                            while(_state != Destroyed && _nextAction == None && wakeTime > now)
                            {
                                _condVar.wait_for(lock, wakeTime - now);
                                now = std::chrono::steady_clock::now();
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
                    assert(timeout != 0s);
                    using namespace std::chrono;

                    registry = Ice::uncheckedCast<InternalRegistryPrx>(_registry->ice_timeout(secondsToInt(timeout)));
                    _nextAction = None;
                    _state = InProgress;
                    _condVar.notify_all();
                }

                switch(action)
                {
                case Connect:
                    assert(!session);
                    {
                        session = createSession(registry, timeout);
                    }
                    break;
                case Disconnect:
                    assert(session);
                    destroySession(session);
                    session = std::nullopt;
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

    bool
    isWaitingForCreate()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _state != Destroyed && _state != Connected;
    }

    virtual bool
    waitForCreate()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condVar.wait(lock, [this] { return _state == Destroyed || _state == Connected; });
        return _state != Destroyed;
    }

    void
    tryCreateSession()
    {
        std::lock_guard<std::mutex> lock(_mutex);
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
        _condVar.notify_all();
    }

    void
    waitTryCreateSession(std::chrono::seconds timeout = std::chrono::seconds(0))
    {
        std::unique_lock<std::mutex> lock(_mutex);
        // Wait until the action is executed and the state changes.
        while(_nextAction == Connect || _nextAction == KeepAlive || _state == InProgress)
        {
            using namespace std::chrono_literals;
            if(timeout == 0s)
            {
                _condVar.wait(lock);
            }
            else
            {
                if(_condVar.wait_for(lock, timeout) == std::cv_status::timeout)
                {
                    break;
                }
            }
        }
    }

    void
    destroyActiveSession()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_state == Destroyed || _state == Disconnected)
        {
            return;
        }
        _nextAction = Disconnect;
        _condVar.notify_all();
    }

    bool
    terminateIfDisconnected()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_state != Disconnected)
        {
            return false; // Nothing we can do for now.
        }
        assert(_state != Destroyed);
        _state = Destroyed;
        _nextAction = None;
        _condVar.notify_all();
        return true;
    }

    void
    terminate(bool destroySession = true)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(_state == Destroyed)
        {
            return;
        }
        assert(_state != Destroyed);
        _state = Destroyed;
        _nextAction = destroySession ? Disconnect : None;
        _condVar.notify_all();
    }

    void
    join()
    {
        _thread.join();
    }

    bool
    isDestroyed()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _state == Destroyed;
    }

    std::optional<TPrx>
    getSession()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _session;
    }

    void
    setRegistry(const InternalRegistryPrxPtr& registry)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _registry = registry;
    }

    InternalRegistryPrxPtr
    getRegistry() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _registry;
    }

    virtual std::optional<TPrx> createSession(InternalRegistryPrxPtr&, std::chrono::seconds&) = 0;
    virtual void destroySession(const std::optional<TPrx>&) = 0;
    virtual bool keepAlive(const std::optional<TPrx>&) = 0;

protected:

    InternalRegistryPrxPtr _registry;
    std::shared_ptr<Ice::Logger> _logger;
    std::optional<TPrx> _session;
    State _state;
    Action _nextAction;

    mutable std::mutex _mutex;
    std::condition_variable _condVar;
    std::thread _thread;
};

class SessionManager
{
public:

    SessionManager(const std::shared_ptr<Ice::Communicator>&, const std::string&);
    virtual ~SessionManager() = default;

    virtual bool isDestroyed() = 0;

protected:

    std::vector<IceGrid::QueryPrxPtr> findAllQueryObjects(bool);

    std::shared_ptr<Ice::Communicator> _communicator;
    std::string _instanceName;
    InternalRegistryPrxPtr _master;
    std::vector<IceGrid::QueryPrxPtr> _queryObjects;

    std::mutex _mutex;
    std::condition_variable _condVar;
};

};

#endif
