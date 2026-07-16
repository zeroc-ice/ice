// Copyright (c) ZeroC, Inc.

#ifndef GLACIER2_INSTANCE_H
#define GLACIER2_INSTANCE_H

#include "Ice/CommunicatorF.h"
#include "Ice/Identity.h"
#include "Ice/ObjectAdapterF.h"
#include "Ice/PropertiesF.h"
#include "Instrumentation.h"
#include "ProxyVerifier.h"
#include "SessionRouterI.h"

#include <string>
#include <vector>

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
        [[nodiscard]] int routingTableMaxSize() const { return _routingTableMaxSize; }
        [[nodiscard]] int routingTableTraceLevel() const { return _routingTableTraceLevel; }
        [[nodiscard]] bool clientForwardContext() const { return _clientForwardContext; }
        [[nodiscard]] bool serverForwardContext() const { return _serverForwardContext; }
        [[nodiscard]] int clientRequestTraceLevel() const { return _clientRequestTraceLevel; }
        [[nodiscard]] int serverRequestTraceLevel() const { return _serverRequestTraceLevel; }
        [[nodiscard]] int clientRejectTraceLevel() const { return _clientRejectTraceLevel; }
        [[nodiscard]] int addConnectionContext() const { return _addConnectionContext; }

        // The session filter configuration, parsed from the Glacier2.Filter.* properties. Each session
        // seeds its own filters from these values.
        [[nodiscard]] const std::vector<std::string>& filterCategories() const { return _filterCategories; }
        [[nodiscard]] const std::vector<std::string>& filterAdapterIds() const { return _filterAdapterIds; }
        [[nodiscard]] const std::vector<Ice::Identity>& filterIdentities() const { return _filterIdentities; }
        [[nodiscard]] int filterAddUserMode() const { return _filterAddUserMode; }

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
        const int _routingTableMaxSize;
        const int _routingTableTraceLevel;
        const bool _clientForwardContext;
        const bool _serverForwardContext;
        const int _clientRequestTraceLevel;
        const int _serverRequestTraceLevel;
        const int _clientRejectTraceLevel;
        const int _addConnectionContext;
        const std::shared_ptr<ProxyVerifier> _proxyVerifier;
        const std::vector<std::string> _filterCategories;
        const std::vector<std::string> _filterAdapterIds;
        const std::vector<Ice::Identity> _filterIdentities;
        const int _filterAddUserMode;
        std::shared_ptr<SessionRouterI> _sessionRouter;
        const std::shared_ptr<Glacier2::Instrumentation::RouterObserver> _observer;
    };

} // End namespace Glacier2

#endif
