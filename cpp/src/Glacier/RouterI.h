// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef GLACIER_ROUTER_I_H
#define GLACIER_ROUTER_I_H

#include <Ice/RoutingTableF.h>
#include <Ice/Ice.h>
#include <Glacier/Router.h>
#include <Glacier/SessionManagerF.h>

namespace Glacier
{

class RouterI : public Router
{
public:

    RouterI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const ::IceInternal::RoutingTablePtr&,
	    const SessionManagerPrx&, const std::string&);
	    
    virtual ~RouterI();

    void destroy();

    virtual Ice::ObjectPrx getClientProxy(const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&) const;
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);
    virtual SessionPrx createSession(const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _clientAdapter;
    Ice::ObjectAdapterPtr _serverAdapter;
    Ice::LoggerPtr _logger;
    ::IceInternal::RoutingTablePtr _routingTable;
    int _routingTableTraceLevel;

    IceUtil::Mutex _sessionMutex;
    SessionManagerPrx _sessionManager;
    std::vector<SessionPrx> _sessions;
    std::string _userId;
};

}

#endif
