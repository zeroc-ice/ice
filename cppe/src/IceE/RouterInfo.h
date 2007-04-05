// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#ifndef ICEE_PURE_CLIENT
#    include <IceE/ObjectAdapterF.h>
#endif
#include <IceE/EndpointF.h>
#include <IceE/Shared.h>
#include <IceE/Mutex.h>

#include <set>

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
    RouterInfoPtr erase(const Ice::RouterPrx&);

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
    std::vector<IceInternal::EndpointPtr> getClientEndpoints();
    std::vector<IceInternal::EndpointPtr> getServerEndpoints();
    void addProxy(const Ice::ObjectPrx&);
#ifndef ICEE_PURE_CLIENT
    void setAdapter(const Ice::ObjectAdapterPtr&);
    Ice::ObjectAdapterPtr getAdapter() const;
#endif

private:

    const Ice::RouterPrx _router;
    std::vector<IceInternal::EndpointPtr> _clientEndpoints;
    std::vector<IceInternal::EndpointPtr> _serverEndpoints;
#ifndef ICEE_PURE_CLIENT
    Ice::ObjectAdapterPtr _adapter;
#endif
    std::set<Ice::Identity> _identities;
};

}

#endif

#endif
