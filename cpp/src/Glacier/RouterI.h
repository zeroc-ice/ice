// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GLACIER_ROUTER_I_H
#define GLACIER_ROUTER_I_H

#include <Ice/RoutingTableF.h>
#include <Ice/Router.h>
#include <Ice/Ice.h>

namespace Glacier
{

class RouterI : public Ice::Router
{
public:

    RouterI(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const ::IceInternal::RoutingTablePtr&);
    virtual ~RouterI();

    void destroy();
    virtual Ice::ObjectPrx getClientProxy(const Ice::Current&);
    virtual Ice::ObjectPrx getServerProxy(const Ice::Current&);
    virtual void addProxy(const Ice::ObjectPrx&, const Ice::Current&);

private:

    Ice::ObjectAdapterPtr _clientAdapter;
    Ice::ObjectAdapterPtr _serverAdapter;
    Ice::LoggerPtr _logger;
    ::IceInternal::RoutingTablePtr _routingTable;
    int _routingTableTraceLevel;
};

}

#endif
