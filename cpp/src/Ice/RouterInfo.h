// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ROUTER_INFO_H
#define ICE_ROUTER_INFO_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/RouterInfoF.h>
#include <Ice/RouterF.h>
#include <Ice/ProxyF.h>
#include <Ice/EndpointIF.h>

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
    std::vector<IceInternal::EndpointIPtr> getClientEndpoints();
    std::vector<IceInternal::EndpointIPtr> getServerEndpoints();
    void addProxy(const Ice::ObjectPrx&);
    void setAdapter(const Ice::ObjectAdapterPtr&);
    Ice::ObjectAdapterPtr getAdapter() const;

private:

    const Ice::RouterPrx _router;
    std::vector<IceInternal::EndpointIPtr> _clientEndpoints;
    std::vector<IceInternal::EndpointIPtr> _serverEndpoints;
    Ice::ObjectAdapterPtr _adapter;
    std::set<Ice::Identity> _identities;
};

}

#endif
