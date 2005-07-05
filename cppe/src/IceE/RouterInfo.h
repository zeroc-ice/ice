// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTER_INFO_H
#define ICEE_ROUTER_INFO_H

#include <IceE/Config.h>

#ifndef ICEE_NO_ROUTER

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/RouterInfoF.h>
#include <IceE/RouterF.h>
#include <IceE/ProxyF.h>
#include <IceE/RoutingTableF.h>

namespace IceEInternal
{

class RouterManager : public IceE::Shared, public IceE::Mutex
{
public:

    RouterManager();

    void destroy();

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    RouterInfoPtr get(const IceE::RouterPrx&);

private:

    std::map<IceE::RouterPrx, RouterInfoPtr> _table;
    std::map<IceE::RouterPrx, RouterInfoPtr>::iterator _tableHint;
};

class RouterInfo : public IceE::Shared, public IceE::Mutex
{
public:

    RouterInfo(const IceE::RouterPrx&);

    void destroy();

    bool operator==(const RouterInfo&) const;
    bool operator!=(const RouterInfo&) const;
    bool operator<(const RouterInfo&) const;

    IceE::RouterPrx getRouter() const;
    IceE::ObjectPrx getClientProxy();
    void setClientProxy(const IceE::ObjectPrx&);
    IceE::ObjectPrx getServerProxy();
    void setServerProxy(const IceE::ObjectPrx&);
    void addProxy(const IceE::ObjectPrx&);
    void setAdapter(const IceE::ObjectAdapterPtr&);
    IceE::ObjectAdapterPtr getAdapter() const;

private:

    const IceE::RouterPrx _router;
    IceE::ObjectPrx _clientProxy;
    IceE::ObjectPrx _serverProxy;
    const RoutingTablePtr _routingTable;
    IceE::ObjectAdapterPtr _adapter;
};

}

#endif

#endif
