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

    std::vector<EndpointIPtr> getClientEndpoints();

    void getClientEndpointsAsync(
        std::function<void(std::vector<EndpointIPtr>)> response,
        std::function<void(std::exception_ptr)> ex);

    std::vector<EndpointIPtr> getServerEndpoints();

    bool addProxyAsync(
        const ReferencePtr& proxy,
        std::function<void()> response,
        std::function<void(std::exception_ptr)> ex);

    void setAdapter(const Ice::ObjectAdapterPtr&);
    Ice::ObjectAdapterPtr getAdapter() const;

    void clearCache(const ReferencePtr&);

private:

    void addAndEvictProxies(const Ice::Identity&, const Ice::ObjectProxySeq&);
    std::vector<EndpointIPtr> setClientEndpoints(const Ice::ObjectPrxPtr&, bool);

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
