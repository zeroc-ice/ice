// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTER_INFO_H
#define ICEE_ROUTER_INFO_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_ROUTER

#include <IceE/RouterInfoF.h>
#include <IceE/RouterF.h>
#include <IceE/RoutingTableF.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/ObjectAdapterF.h>
#endif
#include <IceE/Shared.h>
#include <IceE/Mutex.h>

namespace IceInternal
{

class RouterManager : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    RouterManager();

    void destroy();

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    RouterInfoPtr get(const Ice::RouterPrx&);

private:

    std::map<Ice::RouterPrx, RouterInfoPtr> _table;
    std::map<Ice::RouterPrx, RouterInfoPtr>::iterator _tableHint;
};

class RouterInfo : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    RouterInfo(const Ice::RouterPrx&);

    void destroy();

    bool operator==(const RouterInfo&) const;
    bool operator!=(const RouterInfo&) const;
    bool operator<(const RouterInfo&) const;

    Ice::RouterPrx getRouter() const;
    Ice::ObjectPrx getClientProxy();
    void setClientProxy(const Ice::ObjectPrx&);
    Ice::ObjectPrx getServerProxy();
    void setServerProxy(const Ice::ObjectPrx&);
    void addProxy(const Ice::ObjectPrx&);
#ifndef ICEE_PURE_CLIENT
    void setAdapter(const Ice::ObjectAdapterPtr&);
    Ice::ObjectAdapterPtr getAdapter() const;
#endif

private:

    const Ice::RouterPrx _router;
    Ice::ObjectPrx _clientProxy;
    Ice::ObjectPrx _serverProxy;
    const RoutingTablePtr _routingTable;
#ifndef ICEE_PURE_CLIENT
    Ice::ObjectAdapterPtr _adapter;
#endif
};

}

#endif

#endif
