// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_ROUTER_I_H
#define GLACIER2_ROUTER_I_H

#include "ClientBlobject.h"
#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "ServerBlobject.h"

namespace Glacier2
{
    class RoutingTable;
    class RouterI;
    class FilterManager;

    class RouterI final : public AsyncRouter
    {
    public:
        RouterI(
            std::shared_ptr<Instance>,
            Ice::ConnectionPtr,
            std::string,
            std::optional<SessionPrx>,
            Ice::Identity,
            std::shared_ptr<FilterManager>,
            const Ice::Context&);

        void destroy(std::function<void(std::exception_ptr)>);

        void getClientProxyAsync(
            std::function<void(const std::optional<Ice::ObjectPrx>& returnValue, std::optional<bool> hasRoutingTable)>
                response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void getServerProxyAsync(
            std::function<void(const std::optional<Ice::ObjectPrx>& returnValue)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void addProxiesAsync(
            Ice::ObjectProxySeq,
            std::function<void(const Ice::ObjectProxySeq&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void getCategoryForClientAsync(
            std::function<void(std::string_view)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void createSessionAsync(
            std::string,
            std::string,
            std::function<void(const std::optional<SessionPrx>& returnValue)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void createSessionFromSecureConnectionAsync(
            std::function<void(const std::optional<SessionPrx>& returnValue)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void refreshSessionAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final
        {
            response();
        }

        void destroySessionAsync(
            std::function<void()> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) final;

        void getSessionTimeoutAsync(
            std::function<void(std::int64_t)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        void getACMTimeoutAsync(
            std::function<void(std::int32_t)> response,
            std::function<void(std::exception_ptr)> exception,
            const Ice::Current&) const final;

        std::optional<Ice::ObjectPrx> getServerProxy() const;

        [[nodiscard]] std::shared_ptr<ClientBlobject> getClientBlobject() const;
        [[nodiscard]] std::shared_ptr<ServerBlobject> getServerBlobject() const;

        [[nodiscard]] std::optional<SessionPrx> getSession() const;

        void updateObserver(const std::shared_ptr<Glacier2::Instrumentation::RouterObserver>&);

        [[nodiscard]] std::string toString() const;

        [[nodiscard]] const std::optional<Ice::ObjectPrx>& serverProxy() const noexcept { return _serverProxy; }

    private:
        const std::shared_ptr<Instance> _instance;
        const std::shared_ptr<RoutingTable> _routingTable;
        const std::optional<Ice::ObjectPrx> _serverProxy;
        const std::shared_ptr<ClientBlobject> _clientBlobject;
        const std::shared_ptr<ServerBlobject> _serverBlobject;
        const Ice::ConnectionPtr _connection;
        const std::string _userId;
        const std::optional<SessionPrx> _session;
        const Ice::Identity _controlId;
        const Ice::Context _context;

        std::shared_ptr<Glacier2::Instrumentation::SessionObserver> _observer;
    };
}

#endif
