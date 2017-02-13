// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_INSTANCE_H
#define GLACIER2_INSTANCE_H

#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/PropertiesF.h>
#include <IceUtil/Time.h>

#include <Glacier2/RequestQueue.h>
#include <Glacier2/ProxyVerifier.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/Instrumentation.h>

namespace Glacier2
{

class Instance : public IceUtil::Shared
{
public:

    Instance(const Ice::CommunicatorPtr&, const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&);
    ~Instance();

    Ice::CommunicatorPtr communicator() const { return _communicator; }
    Ice::ObjectAdapterPtr clientObjectAdapter() const { return _clientAdapter; }
    Ice::ObjectAdapterPtr serverObjectAdapter() const { return _serverAdapter; }
    Ice::PropertiesPtr properties() const { return _properties; }
    Ice::LoggerPtr logger() const { return _logger; }

    RequestQueueThreadPtr clientRequestQueueThread() const { return _clientRequestQueueThread; }
    RequestQueueThreadPtr serverRequestQueueThread() const { return _serverRequestQueueThread; }
    ProxyVerifierPtr proxyVerifier() const { return _proxyVerifier; }
    SessionRouterIPtr sessionRouter() const { return _sessionRouter; }

    const Glacier2::Instrumentation::RouterObserverPtr& getObserver() const { return _observer; }

    void destroy();
    
private:

    friend class SessionRouterI;
    void setSessionRouter(const SessionRouterIPtr&);

    const Ice::CommunicatorPtr _communicator;
    const Ice::PropertiesPtr _properties;
    const Ice::LoggerPtr _logger;
    const Ice::ObjectAdapterPtr _clientAdapter;
    const Ice::ObjectAdapterPtr _serverAdapter;
    const RequestQueueThreadPtr _clientRequestQueueThread;
    const RequestQueueThreadPtr _serverRequestQueueThread;
    const ProxyVerifierPtr _proxyVerifier;
    const SessionRouterIPtr _sessionRouter;
    const Glacier2::Instrumentation::RouterObserverPtr _observer;
};
typedef IceUtil::Handle<Instance> InstancePtr;

} // End namespace Glacier2

#endif
