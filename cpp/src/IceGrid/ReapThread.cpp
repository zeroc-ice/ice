// Copyright (c) ZeroC, Inc.

#include "ReapThread.h"
#include "Ice/Ice.h"

using namespace std;
using namespace IceGrid;

ReapThread::ReapThread()
    : _closeCallback([this](const auto& con) { connectionClosed(con); }),
      _thread([this] { run(); })
{
}

void
ReapThread::run()
{
    vector<ReapableItem> reap;
    while (true)
    {
        {
            unique_lock lock(_mutex);
            if (_terminated)
            {
                break;
            }

            calcWakeInterval();

            //
            // If the wake interval is zero then we wait forever.
            //
            if (_wakeInterval == 0s)
            {
                _condVar.wait(lock);
            }
            else
            {
                _condVar.wait_for(lock, _wakeInterval);
            }

            if (_terminated)
            {
                break;
            }

            auto p = _sessions.begin();
            while (p != _sessions.end())
            {
                if (auto timestamp = p->item->timestamp())
                {
                    if (p->timeout > 0s && (chrono::steady_clock::now() - *timestamp > p->timeout))
                    {
                        reap.push_back(*p);
                    }
                    else
                    {
                        ++p;
                        continue; // while loop
                    }
                }
                // else session is already destroyed and we clean-up

                // Remove the reapable
                if (p->connection)
                {
                    auto q = _connections.find(p->connection);
                    if (q != _connections.end())
                    {
                        q->second.erase(p->item);
                        if (q->second.empty())
                        {
                            p->connection->setCloseCallback(nullptr);
                            _connections.erase(q);
                        }
                    }
                }
                p = _sessions.erase(p);
            }
        }

        for (const auto& r : reap)
        {
            r.item->destroy(false);
        }
        reap.clear();
    }
}

void
ReapThread::terminate()
{
    list<ReapableItem> reap;
    {
        lock_guard lock(_mutex);
        if (_terminated)
        {
            assert(_sessions.empty());
            return;
        }
        _terminated = true;
        _condVar.notify_one();
        reap.swap(_sessions);

        for (const auto& conn : _connections)
        {
            conn.first->setCloseCallback(nullptr);
        }
        _connections.clear();
        _closeCallback = nullptr;
    }

    for (const auto& r : reap)
    {
        r.item->destroy(true);
    }
}

void
ReapThread::join()
{
    _thread.join();
}

void
ReapThread::add(const shared_ptr<Reapable>& reapable, chrono::seconds timeout, const Ice::ConnectionPtr& connection)
{
    lock_guard lock(_mutex);
    if (_terminated)
    {
        return;
    }

    // The timeout is 0s (public session timeouts) or >= 10s (node session timeout, replica session timeout).
    assert(timeout == 0s || timeout >= 10s);

    // Registering a reapable with a 0s timeout is allowed. The reapable is reaped only when the reaper thread is
    // shutdown or the connection is closed (when connection is not null).

    _sessions.push_back({reapable, connection, timeout});

    if (connection)
    {
        assert(timeout == 0s);

        auto p = _connections.find(connection);
        if (p == _connections.end())
        {
            // Disable the inactivity check on this connection since it's bound to a session.
            // This is useful for incoming connections managed by the IceGrid.Registry.Client object adapter; for
            // other object adapters, the inactivity timeout is 0 and the check is already disabled.
            connection->disableInactivityCheck();

            p = _connections.insert({connection, {}}).first;
            connection->setCloseCallback(_closeCallback);
        }
        p->second.insert(reapable);
    }

    if (timeout > 0s)
    {
        // If there is a new minimum wake interval then wake the reaping thread.
        if (calcWakeInterval())
        {
            _condVar.notify_one();
        }

        // Since we just added a new session with a non-zero timeout there must be a non-zero wakeInterval.
        assert(_wakeInterval != 0s);
    }
}

void
ReapThread::connectionClosed(const Ice::ConnectionPtr& con)
{
    lock_guard lock(_mutex);
    auto p = _connections.find(con);
    if (p != _connections.end())
    {
        for (const auto& reapable : p->second)
        {
            reapable->destroy(false);
        }
        _connections.erase(p);
    }
}

//
// Returns true if the calculated wake interval is less than the current wake
// interval (or if the original wake interval was "forever").
//
bool
ReapThread::calcWakeInterval()
{
    // Re-calculate minimum timeout
    auto oldWakeInterval = _wakeInterval;
    chrono::milliseconds minimum = 0s;
    bool first = true;
    for (const auto& session : _sessions)
    {
        if (session.timeout != 0s && (first || session.timeout < minimum))
        {
            minimum = session.timeout;
            first = false;
        }
    }

    _wakeInterval = minimum;
    return oldWakeInterval == 0s || minimum < oldWakeInterval;
}
