//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_REAPER_THREAD_H
#define ICEGRID_REAPER_THREAD_H

#include <Ice/Logger.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Connection.h>

#include <list>

namespace IceGrid
{

class Reapable
{
public:

    virtual ~Reapable() = default;

    virtual void heartbeat() const { };

    virtual std::chrono::steady_clock::time_point timestamp() const = 0;
    virtual void destroy(bool) = 0;

};

template<class T>
class SessionReapable : public Reapable
{
public:

    SessionReapable(const std::shared_ptr<Ice::Logger>& logger, const std::shared_ptr<T>& session) :
        _logger(logger), _session(session)
    {
    }

    std::chrono::steady_clock::time_point
    timestamp() const override
    {
        return _session->timestamp();
    }

    void
    destroy(bool shutdown) override
    {
        try
        {
            if(shutdown)
            {
                _session->shutdown();
            }
            else
            {
                _session->destroy(Ice::Current());
            }
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Warning out(_logger);
            out << "unexpected exception while reaping session:\n" << ex;
        }
    }

protected:

    const std::shared_ptr<Ice::Logger> _logger;
    const std::shared_ptr<T> _session;
};

template<class T>
class SessionReapableWithHeartbeat final : public SessionReapable<T>
{
public:

    SessionReapableWithHeartbeat(const std::shared_ptr<Ice::Logger>& logger, const std::shared_ptr<T>& session) :
        SessionReapable<T>(logger, session)
    {
    }

    void
    heartbeat() const override
    {
        try
        {
            SessionReapable<T>::_session->keepAlive(Ice::Current());
        }
        catch(Ice::Exception&)
        {
        }
    }
};

class ReapThread final
{
public:

    ReapThread();

    void terminate();
    void join();
    void add(const std::shared_ptr<Reapable>&, std::chrono::seconds, const std::shared_ptr<Ice::Connection>& = nullptr);

    void connectionHeartbeat(const std::shared_ptr<Ice::Connection>&);
    void connectionClosed(const std::shared_ptr<Ice::Connection>&);

private:

    void run();

    bool calcWakeInterval();

    Ice::CloseCallback _closeCallback;
    Ice::HeartbeatCallback _heartbeatCallback;
    std::chrono::milliseconds _wakeInterval;
    bool _terminated;
    struct ReapableItem
    {
        std::shared_ptr<Reapable> item;
        std::shared_ptr<Ice::Connection> connection;
        std::chrono::milliseconds timeout;
    };
    std::list<ReapableItem> _sessions;

    std::map<std::shared_ptr<Ice::Connection>, std::set<std::shared_ptr<Reapable>> > _connections;

    std::mutex _mutex;
    std::condition_variable _condVar;
    std::thread _thread;
};

}

#endif
