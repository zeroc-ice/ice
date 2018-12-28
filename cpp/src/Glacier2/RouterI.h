// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

class RoutingTable;
typedef IceUtil::Handle<RoutingTable> RoutingTablePtr;

class RouterI;
ICE_DEFINE_PTR(RouterIPtr, RouterI);

class FilterManager;
typedef IceUtil::Handle<FilterManager> FilterManagerPtr;

class RouterI : public Router
{
public:

    RouterI(const InstancePtr&, const Ice::ConnectionPtr&, const std::string&, const SessionPrxPtr&, const Ice::Identity&,
            const FilterManagerPtr&, const Ice::Context&);

    virtual ~RouterI();

    void destroy(const Callback_Session_destroyPtr&);

    virtual Ice::ObjectPrxPtr getClientProxy(IceUtil::Optional<bool>&, const Ice::Current&) const ICE_OVERRIDE;
    virtual Ice::ObjectPrxPtr getServerProxy(const Ice::Current&) const ICE_OVERRIDE;
    virtual Ice::ObjectProxySeq addProxies(const Ice::ObjectProxySeq&, const Ice::Current&) ICE_OVERRIDE;
    virtual std::string getCategoryForClient(const Ice::Current&) const ICE_OVERRIDE;
    virtual void createSession_async(const AMD_Router_createSessionPtr&, const std::string&, const std::string&,
                                     const Ice::Current&);
    virtual void createSessionFromSecureConnection_async(const AMD_Router_createSessionFromSecureConnectionPtr&,
                                                         const Ice::Current&);
    virtual void refreshSession_async(const AMD_Router_refreshSessionPtr&, const ::Ice::Current&);
    virtual void destroySession(const ::Ice::Current&) ICE_OVERRIDE;
    virtual Ice::Long getSessionTimeout(const ::Ice::Current&) const ICE_OVERRIDE;
    virtual Ice::Int getACMTimeout(const ::Ice::Current&) const ICE_OVERRIDE;

    ClientBlobjectPtr getClientBlobject() const;
    ServerBlobjectPtr getServerBlobject() const;

    SessionPrxPtr getSession() const;

    IceUtil::Time getTimestamp() const;
    void updateTimestamp() const;

    void updateObserver(const Glacier2::Instrumentation::RouterObserverPtr&);

    std::string toString() const;

private:

    const InstancePtr _instance;
    const RoutingTablePtr _routingTable;
    const Ice::ObjectPrxPtr _clientProxy;
    const Ice::ObjectPrxPtr _serverProxy;
    const ClientBlobjectPtr _clientBlobject;
    const ServerBlobjectPtr _serverBlobject;
    const bool _clientBlobjectBuffered;
    const bool _serverBlobjectBuffered;
    const Ice::ConnectionPtr _connection;
    const std::string _userId;
    const SessionPrxPtr _session;
    const Ice::Identity _controlId;
    const Ice::Context _context;
    const IceUtil::Mutex _timestampMutex;
    mutable IceUtil::Time _timestamp;

    Glacier2::Instrumentation::SessionObserverPtr _observer;
};

}

#endif
