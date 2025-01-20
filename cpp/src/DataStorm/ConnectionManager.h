// Copyright (c) ZeroC, Inc.

#ifndef DATASTORM_CONNECTION_MANAGER_H
#define DATASTORM_CONNECTION_MANAGER_H

#include "DataStorm/Config.h"
#include "Ice/Ice.h"

namespace DataStormI
{
    class CallbackExecutor;

    class ConnectionManager final : public std::enable_shared_from_this<ConnectionManager>
    {
    public:
        ConnectionManager(const std::shared_ptr<CallbackExecutor>&);

        void
        add(const Ice::ConnectionPtr&,
            std::shared_ptr<void>,
            std::function<void(const Ice::ConnectionPtr&, std::exception_ptr)>);

        void remove(const std::shared_ptr<void>&, const Ice::ConnectionPtr&);
        void remove(const Ice::ConnectionPtr&) noexcept;

        void destroy();

    private:
        using Callback = std::function<void(const Ice::ConnectionPtr&, std::exception_ptr)>;

        std::mutex _mutex;
        std::map<Ice::ConnectionPtr, std::map<std::shared_ptr<void>, Callback>> _connections;
        std::shared_ptr<CallbackExecutor> _executor;
    };
}

#endif
