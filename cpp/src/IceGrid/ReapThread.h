//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_REAPER_THREAD_H
#define ICEGRID_REAPER_THREAD_H

#include "Ice/Connection.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"

#include <list>
#include <set>

namespace IceGrid
{
    class Reapable
    {
    public:
        virtual ~Reapable() = default;

        virtual void heartbeat() const {};

        virtual std::chrono::steady_clock::time_point timestamp() const = 0;
        virtual void destroy(bool) = 0;
    };

    template<class T> class SessionReapable : public Reapable
    {
    public:
        SessionReapable(const Ice::LoggerPtr& logger, const std::shared_ptr<T>& session)
            : _logger(logger),
              _session(session)
        {
        }

        std::chrono::steady_clock::time_point timestamp() const override { return _session->timestamp(); }

        void destroy(bool shutdown) override
        {
            try
            {
                if (shutdown)
                {
                    _session->shutdown();
                }
                else
                {
                    _session->destroy(Ice::Current());
                }
            }
            catch (const Ice::ObjectNotExistException&)
            {
            }
            catch (const std::exception& ex)
            {
                Ice::Warning out(_logger);
                out << "unexpected exception while reaping session:\n" << ex;
            }
        }

    protected:
        const Ice::LoggerPtr _logger;
        const std::shared_ptr<T> _session;
    };

    template<class T> class SessionReapableWithHeartbeat final : public SessionReapable<T>
    {
    public:
        SessionReapableWithHeartbeat(const Ice::LoggerPtr& logger, const std::shared_ptr<T>& session)
            : SessionReapable<T>(logger, session)
        {
        }

        void heartbeat() const override
        {
            try
            {
                SessionReapable<T>::_session->keepAlive(Ice::Current());
            }
            catch (Ice::Exception&)
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
        void add(const std::shared_ptr<Reapable>&, std::chrono::seconds, const Ice::ConnectionPtr& = nullptr);

        void connectionHeartbeat(const Ice::ConnectionPtr&);
        void connectionClosed(const Ice::ConnectionPtr&);

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
            Ice::ConnectionPtr connection;
            std::chrono::milliseconds timeout;
        };
        std::list<ReapableItem> _sessions;

        std::map<Ice::ConnectionPtr, std::set<std::shared_ptr<Reapable>>> _connections;

        std::mutex _mutex;
        std::condition_variable _condVar;
        std::thread _thread;
    };
}

#endif
