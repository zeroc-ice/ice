// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/DisableWarnings.h>

namespace Glacier2
{

class RouterI;
typedef IceUtil::Handle<RouterI> RouterIPtr;

class FilterManager;
typedef IceUtil::Handle<FilterManager> FilterManagerPtr;

class RouterI : public Router, public IceUtil::Mutex
{
public:

    RouterI(const InstancePtr&, const Ice::ConnectionPtr&, const std::string&, const SessionPrx&, const Ice::Identity&,
            const FilterManagerPtr&, const Ice::Context&);
            
    virtual ~RouterI();
    void destroy(const AMI_Session_destroyPtr&);

    virtual Ice::ObjectPrx getClientProxy(const Ice::Current& = Ice::Current()) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current& = Ice::Current()) const;
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);
    virtual Ice::ObjectProxySeq addProxies(const Ice::ObjectProxySeq&, const Ice::Current&);
    virtual std::string getCategoryForClient(const Ice::Current&) const;
    virtual void createSession_async(const AMD_Router_createSessionPtr&, const std::string&, const std::string&, 
                                     const Ice::Current&);
    virtual void createSessionFromSecureConnection_async(const AMD_Router_createSessionFromSecureConnectionPtr&,
                                                         const Ice::Current&);
    virtual void refreshSession(const ::Ice::Current&);
    virtual void destroySession(const ::Ice::Current&);
    virtual Ice::Long getSessionTimeout(const ::Ice::Current&) const;

    ClientBlobjectPtr getClientBlobject() const;
    ServerBlobjectPtr getServerBlobject() const;

    SessionPrx getSession() const;

    IceUtil::Time getTimestamp() const;
    void updateTimestamp() const;

    std::string toString() const;

private:

    const InstancePtr _instance;
    const Ice::ObjectPrx _clientProxy;
    const Ice::ObjectPrx _serverProxy;
    const ClientBlobjectPtr _clientBlobject;
    const ServerBlobjectPtr _serverBlobject;
    const Ice::ConnectionPtr _connection;
    const std::string _userId;
    const SessionPrx _session;
    const Ice::Identity _controlId;
    const Ice::Context _sslContext;
    const IceUtil::Mutex _timestampMutex;
    mutable IceUtil::Time _timestamp;
};

}

#endif
