// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
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
#include <Ice/BuiltinSequences.h>

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

    class GetClientEndpointsCallback : virtual public IceUtil::Shared
    {
    public:
        
        virtual void setEndpoints(const std::vector<EndpointIPtr>&) = 0;
        virtual void setException(const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<GetClientEndpointsCallback> GetClientEndpointsCallbackPtr;

    class AddProxyCallback : virtual public IceUtil::Shared
    {
    public:
        
        virtual void addedProxy() = 0;
        virtual void setException(const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<AddProxyCallback> AddProxyCallbackPtr; 

    RouterInfo(const Ice::RouterPrx&);

    void destroy();

    bool operator==(const RouterInfo&) const;
    bool operator!=(const RouterInfo&) const;
    bool operator<(const RouterInfo&) const;

    Ice::RouterPrx getRouter() const;
    std::vector<EndpointIPtr> getClientEndpoints();
    void getClientEndpoints(const GetClientEndpointsCallbackPtr&);
    std::vector<EndpointIPtr> getServerEndpoints();
    void addProxy(const Ice::ObjectPrx&);
    bool addProxy(const Ice::ObjectPrx&, const AddProxyCallbackPtr&);

    void setAdapter(const Ice::ObjectAdapterPtr&);
    Ice::ObjectAdapterPtr getAdapter() const;

    void clearCache(const ReferencePtr&);

    //
    // The following methods need to be public for access by AMI callbacks.
    //
    std::vector<EndpointIPtr> setClientEndpoints(const Ice::ObjectPrx&);
    std::vector<EndpointIPtr> setServerEndpoints(const Ice::ObjectPrx&);
    void addAndEvictProxies(const Ice::ObjectPrx&, const Ice::ObjectProxySeq&);

private:

    const Ice::RouterPrx _router;
    std::vector<EndpointIPtr> _clientEndpoints;
    std::vector<EndpointIPtr> _serverEndpoints;
    Ice::ObjectAdapterPtr _adapter;
    std::set<Ice::Identity> _identities;
    std::multiset<Ice::Identity> _evictedIdentities;
};

}

#endif
