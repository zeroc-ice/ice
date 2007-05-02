// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_CONNECTION_FACTORY_H
#define ICEE_OUTGOING_CONNECTION_FACTORY_H

#include <IceE/OutgoingConnectionFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/InstanceF.h>
#include <IceE/ObjectAdapterF.h>
#include <IceE/EndpointF.h>
#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterInfoF.h>
#endif
#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <set>

namespace IceInternal
{

class OutgoingConnectionFactory : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void destroy();

    void waitUntilFinished();

    Ice::ConnectionPtr create(const std::vector<EndpointPtr>&);
#ifdef ICEE_HAS_ROUTER
    void setRouterInfo(const RouterInfoPtr&);
#endif
    void removeAdapter(const ::Ice::ObjectAdapterPtr&);
#ifdef ICEE_HAS_BATCH
    void flushBatchRequests();
#endif

private:

    OutgoingConnectionFactory(const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    friend class Instance;

    const InstancePtr _instance;
    bool _destroyed;
    std::multimap<EndpointPtr, Ice::ConnectionPtr> _connections;
    std::set<EndpointPtr> _pending; // Endpoints for which connection establishment is pending.
};

}

#endif
