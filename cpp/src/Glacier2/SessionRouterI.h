// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER2_SESSION_ROUTER_I_H
#define GLACIER2_SESSION_ROUTER_I_H

#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <Ice/Ice.h>
#include <Glacier2/Router.h>

namespace Glacier2
{

class RouterI;
typedef IceUtil::Handle<RouterI> RouterIPtr;

class SessionRouterI;
typedef IceUtil::Handle<SessionRouterI> SessionRouterIPtr;

class SessionRouterI : public Router, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionRouterI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&);
    virtual ~SessionRouterI();
    void destroy();

    virtual Ice::ObjectPrx getClientProxy(const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&) const;
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void createSession(const std::string&, const std::string&, const Ice::Current&);

    RouterIPtr getRouter(const Ice::TransportInfoPtr&) const;    

    virtual void run();

private:

    const Ice::LoggerPtr _logger;
    const Ice::ObjectAdapterPtr _clientAdapter;
    const Ice::ObjectAdapterPtr _serverAdapter;
    const int _traceLevel;

    //
    // TODO: I can't inherit directly from IceUtil::Thread, because of
    // the inheritance of GCShared.
    //
    class SessionThread : public IceUtil::Thread
    {
    public:

	SessionThread(const SessionRouterIPtr&);

	virtual void run();

    private:

	SessionRouterIPtr _sessionRouter;
    };
    const IceUtil::Handle<SessionThread> _sessionThread;

    int _serverAdapterCount;

    std::map<Ice::TransportInfoPtr, RouterIPtr> _routers;
    mutable std::map<Ice::TransportInfoPtr, RouterIPtr>::iterator _routersHint;

    bool _destroy;
};

}

#endif
