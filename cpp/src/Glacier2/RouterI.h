// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_ROUTER_I_H
#define GLACIER2_ROUTER_I_H

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Glacier2/ClientBlobject.h>
#include <Glacier2/ServerBlobject.h>

namespace Glacier2
{

class RouterI;
typedef IceUtil::Handle<RouterI> RouterIPtr;

class RouterI : public Router
{
public:

    RouterI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const Ice::TransportInfoPtr& transport);
    virtual ~RouterI();
    void destroy();

    virtual Ice::ObjectPrx getClientProxy(const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&) const;
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void createSession(const std::string&, const std::string&, const Ice::Current&);

    Glacier2::ClientBlobjectPtr getClientBlobject() const;
    Glacier2::ServerBlobjectPtr getServerBlobject() const;

private:

    const Ice::CommunicatorPtr _communicator;
    const IceInternal::RoutingTablePtr _routingTable;
    const int _routingTableTraceLevel;
    const Ice::ObjectPrx _clientProxy;
    const Ice::ObjectPrx _serverProxy;
    const Glacier2::ClientBlobjectPtr _clientBlobject;
    const Glacier2::ServerBlobjectPtr _serverBlobject;
};

}

#endif
