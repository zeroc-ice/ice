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

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <Glacier2/ClientBlobject.h>
#include <Glacier2/ServerBlobject.h>

namespace Glacier2
{

class ClientRouterI;
typedef IceUtil::Handle<ClientRouterI> ClientRouterIPtr;

class ClientRouterI : public Router
{
public:

    ClientRouterI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const Ice::TransportInfoPtr& transport);
    virtual ~ClientRouterI();
    void destroy();

    virtual Ice::ObjectPrx getClientProxy(const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&) const;
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void createSession(const std::string&, const std::string&, const Ice::Current&);

    Glacier2::ClientBlobjectPtr getClientBlobject() const;
    Glacier2::ServerBlobjectPtr getServerBlobject() const;

private:

    const Ice::LoggerPtr _logger;
    const Ice::ObjectAdapterPtr _clientAdapter;
    const Ice::ObjectAdapterPtr _serverAdapter;
    const IceInternal::RoutingTablePtr _routingTable;
    const int _routingTableTraceLevel;
    const Glacier2::ClientBlobjectPtr _clientBlobject;
    const Glacier2::ServerBlobjectPtr _serverBlobject;
};

class SessionRouterI;
typedef IceUtil::Handle<SessionRouterI> SessionRouterIPtr;

class SessionRouterI : public Router, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionRouterI(const Ice::ObjectAdapterPtr&);
    virtual ~SessionRouterI();
    void destroy();

    virtual Ice::ObjectPrx getClientProxy(const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&) const;
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void createSession(const std::string&, const std::string&, const Ice::Current&);

    ClientRouterIPtr getClientRouter(const Ice::TransportInfoPtr&) const;    

private:

    const Ice::LoggerPtr _logger;
    const Ice::ObjectAdapterPtr _clientAdapter;
    const int _traceLevel;

    int _serverAdapterCount;

    std::map<Ice::TransportInfoPtr, ClientRouterIPtr> _clientRouterMap;
    mutable std::map<Ice::TransportInfoPtr, ClientRouterIPtr>::iterator _clientRouterMapHint;
};

}

#endif
