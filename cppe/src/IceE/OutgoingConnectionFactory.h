// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_CONNECTION_FACTORY_H
#define ICEE_OUTGOING_CONNECTION_FACTORY_H

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/OutgoingConnectionFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/InstanceF.h>
#include <IceE/ObjectAdapterF.h>
#include <IceE/EndpointF.h>
#include <IceE/TransceiverF.h>
#include <IceE/RouterF.h>
#include <set>

namespace IceEInternal
{

class OutgoingConnectionFactory : public IceE::Shared, public IceE::Monitor<IceE::Mutex>
{
public:

    void destroy();

    void waitUntilFinished();

    IceE::ConnectionPtr create(const std::vector<EndpointPtr>&);
#ifndef ICEE_NO_ROUTER
    void setRouter(const ::IceE::RouterPrx&);
#endif
    void removeAdapter(const ::IceE::ObjectAdapterPtr&);
#ifndef ICEE_NO_BATCH
    void flushBatchRequests();
#endif

private:

    OutgoingConnectionFactory(const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    friend class Instance;

    const InstancePtr _instance;
    bool _destroyed;
    std::multimap<EndpointPtr, IceE::ConnectionPtr> _connections;
    std::set<EndpointPtr> _pending; // Endpoints for which connection establishment is pending.
};

}

#endif
