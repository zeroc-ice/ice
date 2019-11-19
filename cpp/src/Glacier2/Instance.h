//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GLACIER2_INSTANCE_H
#define GLACIER2_INSTANCE_H

#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>

#include <Glacier2/RequestQueue.h>
#include <Glacier2/ProxyVerifier.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/Instrumentation.h>

namespace Glacier2
{

class Instance
{
public:

    Instance(std::shared_ptr<Ice::Communicator>, std::shared_ptr<Ice::ObjectAdapter>,
             std::shared_ptr<Ice::ObjectAdapter>);

    std::shared_ptr<Ice::Communicator> communicator() const { return _communicator; }
    std::shared_ptr<Ice::ObjectAdapter> clientObjectAdapter() const { return _clientAdapter; }
    std::shared_ptr<Ice::ObjectAdapter> serverObjectAdapter() const { return _serverAdapter; }
    std::shared_ptr<Ice::Properties> properties() const { return _properties; }
    std::shared_ptr<Ice::Logger> logger() const { return _logger; }

    std::shared_ptr<RequestQueueThread> clientRequestQueueThread() const { return _clientRequestQueueThread; }
    std::shared_ptr<RequestQueueThread> serverRequestQueueThread() const { return _serverRequestQueueThread; }
    std::shared_ptr<ProxyVerifier> proxyVerifier() const { return _proxyVerifier; }
    std::shared_ptr<SessionRouterI> sessionRouter() const { return _sessionRouter; }

    const std::shared_ptr<Glacier2::Instrumentation::RouterObserver>& getObserver() const { return _observer; }

    void setSessionRouter(std::shared_ptr<SessionRouterI>);

    void destroy();

private:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::shared_ptr<Ice::Properties> _properties;
    const std::shared_ptr<Ice::Logger> _logger;
    const std::shared_ptr<Ice::ObjectAdapter> _clientAdapter;
    const std::shared_ptr<Ice::ObjectAdapter> _serverAdapter;
    const std::shared_ptr<RequestQueueThread> _clientRequestQueueThread;
    const std::shared_ptr<RequestQueueThread> _serverRequestQueueThread;
    const std::shared_ptr<ProxyVerifier> _proxyVerifier;
    const std::shared_ptr<SessionRouterI> _sessionRouter;
    const std::shared_ptr<Glacier2::Instrumentation::RouterObserver> _observer;
};

} // End namespace Glacier2

#endif
