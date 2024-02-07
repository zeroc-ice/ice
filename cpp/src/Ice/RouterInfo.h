//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ROUTER_INFO_H
#define ICE_ROUTER_INFO_H

#include <Ice/RouterInfoF.h>
#include <Ice/Router.h>
#include <Ice/ProxyF.h>
#include <Ice/ReferenceF.h>
#include <Ice/EndpointIF.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/Identity.h>
#include <Ice/Comparable.h>

#include <mutex>
#include <set>

namespace IceInternal
{

class RouterManager final
{
public:

    RouterManager();

    void destroy();

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    RouterInfoPtr get(const Ice::RouterPrxPtr&);
    RouterInfoPtr erase(const Ice::RouterPrxPtr&);

private:

    using RouterInfoTable = std::map<std::shared_ptr<Ice::RouterPrx>,
                                     RouterInfoPtr,
                                     Ice::TargetCompare<std::shared_ptr<Ice::RouterPrx>, std::less>>;

    RouterInfoTable _table;
    RouterInfoTable::iterator _tableHint;
    std::mutex _mutex;
};

class RouterInfo final : public std::enable_shared_from_this<RouterInfo>
{
public:

    class GetClientEndpointsCallback
    {
    public:

        virtual void setEndpoints(const std::vector<EndpointIPtr>&) = 0;
        virtual void setException(const Ice::LocalException&) = 0;
    };
    using GetClientEndpointsCallbackPtr = std::shared_ptr<GetClientEndpointsCallback>;

    class AddProxyCallback
    {
    public:

        virtual void addedProxy() = 0;
        virtual void setException(const Ice::LocalException&) = 0;
    };
    using AddProxyCallbackPtr = std::shared_ptr<AddProxyCallback>;

    RouterInfo(const Ice::RouterPrxPtr&);

    void destroy();

    bool operator==(const RouterInfo&) const;
    bool operator<(const RouterInfo&) const;

    const Ice::RouterPrxPtr& getRouter() const
    {
        //
        // No mutex lock necessary, _router is immutable.
        //
        return _router;
    }
    void getClientProxyResponse(const Ice::ObjectPrxPtr&, const std::optional<bool>&,
                                const GetClientEndpointsCallbackPtr&);
    void getClientProxyException(const Ice::Exception&, const GetClientEndpointsCallbackPtr&);
    std::vector<EndpointIPtr> getClientEndpoints();
    void getClientEndpoints(const GetClientEndpointsCallbackPtr&);
    std::vector<EndpointIPtr> getServerEndpoints();

    class AddProxyCookie final
    {
    public:

        AddProxyCookie(const AddProxyCallbackPtr cb, const Ice::ObjectPrxPtr& proxy) :
            _cb(cb),
            _proxy(proxy)
        {
        }

        AddProxyCallbackPtr cb() const
        {
            return _cb;
        }

        Ice::ObjectPrxPtr proxy() const
        {
            return _proxy;
        }

    private:

        const AddProxyCallbackPtr _cb;
        const Ice::ObjectPrxPtr _proxy;
    };
    using AddProxyCookiePtr = std::shared_ptr<AddProxyCookie>;

    void addProxyResponse(const Ice::ObjectProxySeq&, const AddProxyCookiePtr&);
    void addProxyException(const Ice::Exception&, const AddProxyCookiePtr&);
    bool addProxy(const Ice::ObjectPrxPtr&, const AddProxyCallbackPtr&);

    void setAdapter(const Ice::ObjectAdapterPtr&);
    Ice::ObjectAdapterPtr getAdapter() const;

    void clearCache(const ReferencePtr&);

    //
    // The following methods need to be public for access by AMI callbacks.
    //
    std::vector<EndpointIPtr> setClientEndpoints(const Ice::ObjectPrxPtr&, bool);
    void addAndEvictProxies(const Ice::ObjectPrxPtr&, const Ice::ObjectProxySeq&);

private:

    const Ice::RouterPrxPtr _router;
    std::vector<EndpointIPtr> _clientEndpoints;
    bool _hasRoutingTable;
    Ice::ObjectAdapterPtr _adapter;
    std::set<Ice::Identity> _identities;
    std::multiset<Ice::Identity> _evictedIdentities;
    mutable std::mutex _mutex;
};

}

#endif
