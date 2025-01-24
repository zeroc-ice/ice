// Copyright (c) ZeroC, Inc.

#include "RouterInfo.h"
#include "Ice/Connection.h" // For ice_connection()->timeout().
#include "Ice/LocalExceptions.h"
#include "Ice/ProxyFunctions.h"
#include "Ice/Router.h"
#include "Reference.h"

#include <cassert>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::RouterManager::RouterManager() : _tableHint(_table.end()) {}

void
IceInternal::RouterManager::destroy()
{
    lock_guard lock(_mutex);
    for (const auto& [_, routerInfo] : _table)
    {
        routerInfo->destroy();
    }
    _table.clear();
    _tableHint = _table.end();
}

RouterInfoPtr
IceInternal::RouterManager::get(const RouterPrx& rtr)
{
    RouterPrx router = rtr->ice_router(nullopt); // The router cannot be routed.

    lock_guard lock(_mutex);

    auto p = _table.end();

    if (_tableHint != _table.end())
    {
        if (_tableHint->first == router)
        {
            p = _tableHint;
        }
    }

    if (p == _table.end())
    {
        p = _table.find(router);
    }

    if (p == _table.end())
    {
        _tableHint = _table.insert(_tableHint, pair<const RouterPrx, RouterInfoPtr>(router, new RouterInfo(router)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

void
IceInternal::RouterManager::erase(const RouterPrx& router)
{
    assert(!router->ice_getRouter()); // The router cannot be routed.

    lock_guard lock(_mutex);

    auto p = _table.end();
    if (_tableHint != _table.end() && _tableHint->first == router)
    {
        p = _tableHint;
        _tableHint = _table.end();
    }

    if (p == _table.end())
    {
        p = _table.find(router);
    }

    if (p != _table.end())
    {
        _table.erase(p);
    }
}

IceInternal::RouterInfo::RouterInfo(RouterPrx router) : _router(std::move(router)) {}

void
IceInternal::RouterInfo::destroy()
{
    lock_guard lock(_mutex);

    _clientEndpoints.clear();
    _adapter = nullptr;
    _identities.clear();
}

bool
IceInternal::RouterInfo::operator==(const RouterInfo& rhs) const
{
    return _router == rhs._router;
}

bool
IceInternal::RouterInfo::operator<(const RouterInfo& rhs) const
{
    return _router < rhs._router;
}

vector<EndpointIPtr>
IceInternal::RouterInfo::getClientEndpoints()
{
    {
        lock_guard lock(_mutex);
        if (!_clientEndpoints.empty())
        {
            return _clientEndpoints;
        }
    }

    optional<bool> hasRoutingTable;
    optional<ObjectPrx> proxy = _router->getClientProxy(hasRoutingTable);
    return setClientEndpoints(proxy, hasRoutingTable ? hasRoutingTable.value() : true);
}

void
IceInternal::RouterInfo::getClientEndpointsAsync(
    std::function<void(vector<EndpointIPtr>)> response,
    std::function<void(std::exception_ptr)> ex)
{
    vector<EndpointIPtr> clientEndpoints;
    {
        lock_guard lock(_mutex);
        clientEndpoints = _clientEndpoints;
    }

    if (!clientEndpoints.empty())
    {
        response(std::move(clientEndpoints));
        return;
    }

    RouterInfoPtr self = shared_from_this();
    _router->getClientProxyAsync(
        [self, response = std::move(response)](const optional<Ice::ObjectPrx>& proxy, optional<bool> hasRoutingTable)
        { response(self->setClientEndpoints(proxy, hasRoutingTable.value_or(true))); },
        std::move(ex));
}

vector<EndpointIPtr>
IceInternal::RouterInfo::getServerEndpoints()
{
    optional<ObjectPrx> serverProxy = _router->getServerProxy();
    if (!serverProxy)
    {
        throw NoEndpointException{__FILE__, __LINE__, "Router::getServerProxy returned a null proxy"};
    }
    serverProxy = serverProxy->ice_router(nullopt); // The server proxy cannot be routed.
    return serverProxy->_getReference()->getEndpoints();
}

bool
IceInternal::RouterInfo::addProxyAsync(
    const ReferencePtr& reference,
    function<void()> response,
    function<void(exception_ptr)> ex)
{
    assert(reference);
    Identity identity = reference->getIdentity();

    {
        lock_guard lock(_mutex);
        if (!_hasRoutingTable)
        {
            return true; // The router implementation doesn't maintain a routing table.
        }
        else if (_identities.find(identity) != _identities.end())
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return true;
        }
    }

    Ice::ObjectProxySeq proxies;
    proxies.emplace_back(ObjectPrx::_fromReference(reference));

    _router->addProxiesAsync(
        proxies,
        [response = std::move(response), self = shared_from_this(), identity = std::move(identity)](
            const Ice::ObjectProxySeq& evictedProxies)
        {
            self->addAndEvictProxies(identity, evictedProxies);
            response();
        },
        std::move(ex));
    return false;
}

void
IceInternal::RouterInfo::setAdapter(const ObjectAdapterPtr& adapter)
{
    lock_guard lock(_mutex);
    _adapter = adapter;
}

ObjectAdapterPtr
IceInternal::RouterInfo::getAdapter() const
{
    lock_guard lock(_mutex);
    return _adapter;
}

void
IceInternal::RouterInfo::clearCache(const ReferencePtr& ref)
{
    lock_guard lock(_mutex);
    _identities.erase(ref->getIdentity());
}

vector<EndpointIPtr>
IceInternal::RouterInfo::setClientEndpoints(const optional<ObjectPrx>& proxy, bool hasRoutingTable)
{
    lock_guard lock(_mutex);
    if (_clientEndpoints.empty())
    {
        _hasRoutingTable = hasRoutingTable;
        _clientEndpoints = proxy ? proxy->_getReference()->getEndpoints() : _router->_getReference()->getEndpoints();
    }
    return _clientEndpoints;
}

void
IceInternal::RouterInfo::addAndEvictProxies(const Identity& identity, const Ice::ObjectProxySeq& evictedProxies)
{
    lock_guard lock(_mutex);

    //
    // Check if the proxy hasn't already been evicted by a concurrent addProxies call.
    // If it's the case, don't add it to our local map.
    //
    auto p = _evictedIdentities.find(identity);
    if (p != _evictedIdentities.end())
    {
        _evictedIdentities.erase(p);
    }
    else
    {
        //
        // If we successfully added the proxy to the router,
        // we add it to our local map.
        //
        _identities.insert(identity);
    }

    //
    // We also must remove whatever proxies the router evicted.
    //
    for (const auto& evictedProxy : evictedProxies)
    {
        if (_identities.erase(evictedProxy->ice_getIdentity()) == 0)
        {
            //
            // It's possible for the proxy to not have been
            // added yet in the local map if two threads
            // concurrently call addProxies.
            //
            _evictedIdentities.insert(evictedProxy->ice_getIdentity());
        }
    }
}
