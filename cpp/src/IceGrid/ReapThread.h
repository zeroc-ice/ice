// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_REAPER_THREAD_H
#define ICEGRID_REAPER_THREAD_H

#include "Ice/Ice.h"

#include <list>
#include <set>

namespace IceGrid
{
    class Reapable
    {
    public:
        virtual ~Reapable() = default;

        // Returns the timestamp of the most recent keepAlive call, or nullopt if the session is destroyed.
        [[nodiscard]] virtual std::optional<std::chrono::steady_clock::time_point> timestamp() const noexcept = 0;
        virtual void destroy(bool) = 0;
    };

    // We use this template with various Session servants to convert destroy(bool) calls into shutdown() or destroy()
    // on the servant.
    template<class T> class SessionReapable final : public Reapable
    {
    public:
        SessionReapable(Ice::LoggerPtr logger, const std::shared_ptr<T>& session)
            : _logger(std::move(logger)),
              _session(session)
        {
        }

        [[nodiscard]] std::optional<std::chrono::steady_clock::time_point> timestamp() const noexcept final
        {
            return _session->timestamp();
        }

        void destroy(bool shutdown) final
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

    private:
        const Ice::LoggerPtr _logger;
        const std::shared_ptr<T> _session;
    };

    // A shared monitoring/reaping mechanism that destroys session servants. It's used for all session servants hosted
    // by the IceGrid registry: admin sessions, client sessions (aka resource allocation sessions), internal node
    // sessions and internal replica sessions.
    // It supports two modes:
    // - 0s timeout + non-null connection: the session is bound to the connection and is destroyed/reaped either
    // explicitly (via a call to a Slice-defined destroy operation) or when the connection is closed
    // - a null connection with usually a non-0 timeout: the session's lifetime is independent of the connection, and
    // the reaper destroys the session when it doesn't receive a keepAlive call within timeout. If the timeout is 0s,
    // the reaper only reaps the session when it's destroyed explicitly, or when the IceGrid registry shuts down.
    class ReapThread final
    {
    public:
        ReapThread();

        void terminate();
        void join();

        // Add a session (wrapped in a Reapable object), with a specified timeout (can be 0s) and connection (can be
        // null).
        void
        add(const std::shared_ptr<Reapable>& reapable,
            std::chrono::seconds timeout,
            const Ice::ConnectionPtr& connection);

        void connectionClosed(const Ice::ConnectionPtr&);

    private:
        void run();

        bool calcWakeInterval();

        Ice::CloseCallback _closeCallback;
        std::chrono::milliseconds _wakeInterval;
        bool _terminated{false};
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
