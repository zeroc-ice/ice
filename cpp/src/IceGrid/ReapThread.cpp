//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/ReapThread.h>

using namespace std;
using namespace IceGrid;

ReapThread::ReapThread() :
    _closeCallback([this](const auto& con) { connectionClosed(con); }),
    _heartbeatCallback([this](const auto& con) { connectionHeartbeat(con); }),
    _terminated(false),
    _thread([this] { run(); })
{
}

void
ReapThread::run()
{
    vector<ReapableItem> reap;
    while(true)
    {
        {
            unique_lock lock(_mutex);
            if(_terminated)
            {
                break;
            }

            calcWakeInterval();

            //
            // If the wake interval is zero then we wait forever.
            //
            if(_wakeInterval == 0s)
            {
                _condVar.wait(lock);
            }
            else
            {
                _condVar.wait_for(lock, _wakeInterval);
            }

            if(_terminated)
            {
                break;
            }

            auto p = _sessions.begin();
            while(p != _sessions.end())
            {
                try
                {
                    if(p->timeout == 0s)
                    {
                        p->item->timestamp(); // This should throw if the reapable is destroyed.
                        ++p;
                        continue;
                    }
                    else if((chrono::steady_clock::now() - p->item->timestamp()) > p->timeout)
                    {
                        reap.push_back(*p);
                    }
                    else
                    {
                        ++p;
                        continue;
                    }
                }
                catch(const Ice::ObjectNotExistException&)
                {
                }

                //
                // Remove the reapable
                //
                if(p->connection)
                {
                    auto q = _connections.find(p->connection);
                    if(q != _connections.end())
                    {
                        q->second.erase(p->item);
                        if(q->second.empty())
                        {
                            p->connection->setCloseCallback(nullptr);
                            p->connection->setHeartbeatCallback(nullptr);
                            _connections.erase(q);
                        }
                    }
                }
                p = _sessions.erase(p);
            }
        }

        for(const auto& r : reap)
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
        if(_terminated)
        {
            assert(_sessions.empty());
            return;
        }
        _terminated = true;
        _condVar.notify_one();
        reap.swap(_sessions);

        for(const auto& conn : _connections)
        {
            conn.first->setCloseCallback(nullptr);
            conn.first->setHeartbeatCallback(nullptr);
        }
        _connections.clear();
        _closeCallback = nullptr;
        _heartbeatCallback = nullptr;
    }

    for(const auto& r : reap)
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
ReapThread::add(const shared_ptr<Reapable>& reapable, chrono::seconds timeout,
                const shared_ptr<Ice::Connection>& connection)
{
    lock_guard lock(_mutex);
    if(_terminated)
    {
        return;
    }

    //
    // NOTE: registering a reapable with a null timeout is allowed. The reapable is reaped
    // only when the reaper thread is shutdown.
    //

    //
    // 10 seconds is the minimum permissable timeout.
    //
    if(timeout > 0s && timeout < 10s)
    {
        timeout = 10s;
    }

    _sessions.push_back({ reapable, connection, timeout });

    if(connection)
    {
        auto p = _connections.find(connection);
        if(p == _connections.end())
        {
            p = _connections.insert({connection, {} }).first;
            connection->setCloseCallback(_closeCallback);
            connection->setHeartbeatCallback(_heartbeatCallback);
        }
        p->second.insert(reapable);
    }

    if(timeout > 0s)
    {
        //
        // If there is a new minimum wake interval then wake the reaping
        // thread.
        //
        if(calcWakeInterval())
        {
            _condVar.notify_one();
        }

        //
        // Since we just added a new session with a non null timeout there
        // must be a non-zero wakeInterval.
        //
        assert(_wakeInterval != 0s);
    }
}

void
ReapThread::connectionHeartbeat(const shared_ptr<Ice::Connection>& con)
{
    lock_guard lock(_mutex);

    auto p = _connections.find(con);
    if(p == _connections.end())
    {
        con->setCloseCallback(nullptr);
        con->setHeartbeatCallback(nullptr);
        return;
    }

    for(const auto& reapable : p->second)
    {
        reapable->heartbeat();
    }
}

void
ReapThread::connectionClosed(const shared_ptr<Ice::Connection>& con)
{
    lock_guard lock(_mutex);

    auto p = _connections.find(con);
    if(p == _connections.end())
    {
        con->setCloseCallback(nullptr);
        con->setHeartbeatCallback(nullptr);
        return;
    }

    for(const auto& reapable : p->second)
    {
        reapable->destroy(false);
    }
    _connections.erase(p);
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
    for(const auto& session : _sessions)
    {
        if(session.timeout != 0s && (first || session.timeout < minimum))
        {
            minimum = session.timeout;
            first = false;
        }
    }

    _wakeInterval = minimum;
    return oldWakeInterval == 0s || minimum < oldWakeInterval;
}
