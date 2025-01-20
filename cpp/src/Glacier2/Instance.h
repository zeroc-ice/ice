// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_INSTANCE_H
#define GLACIER2_INSTANCE_H

#include "Ice/CommunicatorF.h"
#include "Ice/ObjectAdapterF.h"
#include "Ice/PropertiesF.h"
#include "Instrumentation.h"
#include "ProxyVerifier.h"
#include "SessionRouterI.h"

namespace Glacier2
{
    class Instance
    {
    public:
        Instance(Ice::CommunicatorPtr, Ice::ObjectAdapterPtr, Ice::ObjectAdapterPtr);

        [[nodiscard]] Ice::CommunicatorPtr communicator() const { return _communicator; }
        [[nodiscard]] Ice::ObjectAdapterPtr clientObjectAdapter() const { return _clientAdapter; }
        [[nodiscard]] Ice::ObjectAdapterPtr serverObjectAdapter() const { return _serverAdapter; }
        [[nodiscard]] Ice::PropertiesPtr properties() const { return _properties; }
        [[nodiscard]] Ice::LoggerPtr logger() const { return _logger; }

        [[nodiscard]] std::shared_ptr<ProxyVerifier> proxyVerifier() const { return _proxyVerifier; }
        [[nodiscard]] std::shared_ptr<SessionRouterI> sessionRouter() const { return _sessionRouter; }

        [[nodiscard]] const std::shared_ptr<Glacier2::Instrumentation::RouterObserver>& getObserver() const
        {
            return _observer;
        }

        void setSessionRouter(std::shared_ptr<SessionRouterI>);

        void destroy();

    private:
        const Ice::CommunicatorPtr _communicator;
        const Ice::PropertiesPtr _properties;
        const Ice::LoggerPtr _logger;
        const Ice::ObjectAdapterPtr _clientAdapter;
        const Ice::ObjectAdapterPtr _serverAdapter;
        const std::shared_ptr<ProxyVerifier> _proxyVerifier;
        std::shared_ptr<SessionRouterI> _sessionRouter;
        const std::shared_ptr<Glacier2::Instrumentation::RouterObserver> _observer;
    };

} // End namespace Glacier2

#endif
