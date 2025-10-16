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

    class RouterI final : public Router
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

        std::optional<Ice::ObjectPrx> getClientProxy(std::optional<bool>&, const Ice::Current&) const final;

        [[nodiscard]] std::optional<Ice::ObjectPrx> getServerProxy(const Ice::Current&) const final;

        Ice::ObjectProxySeq addProxies(Ice::ObjectProxySeq, const Ice::Current&) final;

        [[nodiscard]] std::string getCategoryForClient(const Ice::Current&) const final;

        std::optional<SessionPrx> createSession(std::string, std::string, const Ice::Current&) final;

        std::optional<SessionPrx> createSessionFromSecureConnection(const Ice::Current&) final;

        void refreshSession(const Ice::Current&) final {}

        void destroySession(const Ice::Current&) final;

        [[nodiscard]] std::int64_t getSessionTimeout(const Ice::Current&) const final;

        [[nodiscard]] std::int32_t getACMTimeout(const Ice::Current&) const final;

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
