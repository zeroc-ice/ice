// Copyright (c) ZeroC, Inc.

#include "ConnectionManager.h"
#include "CallbackExecutor.h"

#include <iostream>

using namespace std;
using namespace DataStormI;
using namespace Ice;

ConnectionManager::ConnectionManager(const shared_ptr<CallbackExecutor>& executor) : _executor(executor) {}

void
ConnectionManager::add(
    const ConnectionPtr& connection,
    shared_ptr<void> object,
    function<void(const ConnectionPtr&, exception_ptr)> callback)
{
    // Disable the inactivity timeout on the connection.
    connection->disableInactivityCheck();

    lock_guard<mutex> lock(_mutex);
    auto& objects = _connections[connection];
    if (objects.empty())
    {
        connection->setCloseCallback([self = shared_from_this()](const ConnectionPtr& con) { self->remove(con); });
    }
    objects.emplace(std::move(object), std::move(callback));
}

void
ConnectionManager::remove(const shared_ptr<void>& object, const ConnectionPtr& connection)
{
    lock_guard<mutex> lock(_mutex);
    auto p = _connections.find(connection);
    if (p == _connections.end())
    {
        return;
    }
    auto& objects = p->second;
    objects.erase(object);
    if (objects.empty())
    {
        connection->setCloseCallback(nullptr);
        _connections.erase(p);
    }
}

void
ConnectionManager::remove(const ConnectionPtr& connection) noexcept
{
    map<shared_ptr<void>, Callback> objects;
    {
        lock_guard<mutex> lock(_mutex);
        auto p = _connections.find(connection);
        if (p == _connections.end())
        {
            return;
        }
        objects.swap(p->second);
        connection->setCloseCallback(nullptr);
        _connections.erase(p);
    }

    exception_ptr ex;
    try
    {
        connection->throwException();
    }
    catch (const std::exception&)
    {
        ex = current_exception();
    }

    for (const auto& [_, callback] : objects)
    {
        callback(connection, ex);
    }
    _executor->flush();
}

void
ConnectionManager::destroy()
{
    lock_guard<mutex> lock(_mutex);
    for (const auto& connection : _connections)
    {
        connection.first->setCloseCallback(nullptr);
    }
    _connections.clear();
}
