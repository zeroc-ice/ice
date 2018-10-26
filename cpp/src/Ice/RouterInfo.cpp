// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RouterInfo.h>
#include <Ice/Router.h>
#include <Ice/LocalException.h>
#include <Ice/Connection.h> // For ice_connection()->timeout().
#include <Ice/Functional.h>
#include <Ice/Reference.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(RouterManager* p) { return p; }
IceUtil::Shared* IceInternal::upCast(RouterInfo* p) { return p; }

IceInternal::RouterManager::RouterManager() :
    _tableHint(_table.end())
{
}

void
IceInternal::RouterManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);
#ifdef ICE_CPP11_MAPPING
    for_each(_table.begin(), _table.end(), [](const pair<shared_ptr<RouterPrx>, RouterInfoPtr> it){ it.second->destroy(); });
#else
    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<const RouterPrx, RouterInfo>(&RouterInfo::destroy));
#endif
    _table.clear();
    _tableHint = _table.end();
}

RouterInfoPtr
IceInternal::RouterManager::get(const RouterPrxPtr& rtr)
{
    if(!rtr)
    {
        return 0;
    }

    RouterPrxPtr router = rtr->ice_router(0); // The router cannot be routed.

    IceUtil::Mutex::Lock sync(*this);

    RouterInfoTable::iterator p = _table.end();

    if(_tableHint != _table.end())
    {
        if(targetEqualTo(_tableHint->first, router))
        {
            p = _tableHint;
        }
    }

    if(p == _table.end())
    {
        p = _table.find(router);
    }

    if(p == _table.end())
    {
        _tableHint = _table.insert(_tableHint, pair<const RouterPrxPtr, RouterInfoPtr>(router, new RouterInfo(router)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

RouterInfoPtr
IceInternal::RouterManager::erase(const RouterPrxPtr& rtr)
{
    RouterInfoPtr info;
    if(rtr)
    {
        RouterPrxPtr router = ICE_UNCHECKED_CAST(RouterPrx, rtr->ice_router(ICE_NULLPTR)); // The router cannot be routed.
        IceUtil::Mutex::Lock sync(*this);

        RouterInfoTable::iterator p = _table.end();
        if(_tableHint != _table.end() && targetEqualTo(_tableHint->first, router))
        {
            p = _tableHint;
            _tableHint = _table.end();
        }

        if(p == _table.end())
        {
            p = _table.find(router);
        }

        if(p != _table.end())
        {
            info = p->second;
            _table.erase(p);
        }
    }

    return info;
}

IceInternal::RouterInfo::RouterInfo(const RouterPrxPtr& router) : _router(router), _hasRoutingTable(false)
{
    assert(_router);
}

void
IceInternal::RouterInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _clientEndpoints.clear();
    _adapter = 0;
    _identities.clear();
}

bool
IceInternal::RouterInfo::operator==(const RouterInfo& rhs) const
{
    return Ice::targetEqualTo(_router, rhs._router);
}

bool
IceInternal::RouterInfo::operator<(const RouterInfo& rhs) const
{
    return Ice::targetLess(_router, rhs._router);
}

vector<EndpointIPtr>
IceInternal::RouterInfo::getClientEndpoints()
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_clientEndpoints.empty())
        {
            return _clientEndpoints;
        }
    }

    IceUtil::Optional<bool> hasRoutingTable;
    Ice::ObjectPrxPtr proxy = _router->getClientProxy(hasRoutingTable);
    return setClientEndpoints(proxy, hasRoutingTable ? hasRoutingTable.value() : true);
}

void
IceInternal::RouterInfo::getClientProxyResponse(const Ice::ObjectPrxPtr& proxy,
                                                const IceUtil::Optional<bool>& hasRoutingTable,
                                                const GetClientEndpointsCallbackPtr& callback)
{
    callback->setEndpoints(setClientEndpoints(proxy, hasRoutingTable ? hasRoutingTable.value() : true));
}

void
IceInternal::RouterInfo::getClientProxyException(const Ice::Exception& ex,
                                                 const GetClientEndpointsCallbackPtr& callback)
{
    callback->setException(dynamic_cast<const Ice::LocalException&>(ex));
}

void
IceInternal::RouterInfo::getClientEndpoints(const GetClientEndpointsCallbackPtr& callback)
{
    vector<EndpointIPtr> clientEndpoints;
    {
        IceUtil::Mutex::Lock sync(*this);
        clientEndpoints = _clientEndpoints;
    }

    if(!clientEndpoints.empty())
    {
        callback->setEndpoints(clientEndpoints);
        return;
    }

#ifdef ICE_CPP11_MAPPING
    RouterInfoPtr self = this;
    _router->getClientProxyAsync(
        [self, callback](const Ice::ObjectPrxPtr& proxy, Ice::optional<bool> hasRoutingTable)
        {
            self->getClientProxyResponse(proxy, hasRoutingTable, callback);
        },
        [self, callback](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::Exception& ex)
            {
                self->getClientProxyException(ex, callback);
            }
        });
#else
    _router->begin_getClientProxy(newCallback_Router_getClientProxy(this,
                                                                    &RouterInfo::getClientProxyResponse,
                                                                    &RouterInfo::getClientProxyException),
                                  callback);
#endif
}

vector<EndpointIPtr>
IceInternal::RouterInfo::getServerEndpoints()
{
    Ice::ObjectPrxPtr serverProxy = _router->getServerProxy();
    if(!serverProxy)
    {
        throw NoEndpointException(__FILE__, __LINE__);
    }
    serverProxy = serverProxy->ice_router(0); // The server proxy cannot be routed.
    return serverProxy->_getReference()->getEndpoints();
}

void
IceInternal::RouterInfo::addProxyResponse(const Ice::ObjectProxySeq& proxies, const AddProxyCookiePtr& cookie)
{
    addAndEvictProxies(cookie->proxy(), proxies);
    cookie->cb()->addedProxy();
}

void
IceInternal::RouterInfo::addProxyException(const Ice::Exception& ex, const AddProxyCookiePtr& cookie)
{
    cookie->cb()->setException(dynamic_cast<const Ice::LocalException&>(ex));
}

bool
IceInternal::RouterInfo::addProxy(const Ice::ObjectPrxPtr& proxy, const AddProxyCallbackPtr& callback)
{
    assert(proxy);
    {
        IceUtil::Mutex::Lock sync(*this);
        if(!_hasRoutingTable)
        {
            return true; // The router implementation doesn't maintain a routing table.
        }
        else if(_identities.find(proxy->ice_getIdentity()) != _identities.end())
        {
            //
            // Only add the proxy to the router if it's not already in our local map.
            //
            return true;
        }
    }

    Ice::ObjectProxySeq proxies;
    proxies.push_back(proxy);
    AddProxyCookiePtr cookie = new AddProxyCookie(callback, proxy);

#ifdef ICE_CPP11_MAPPING
    RouterInfoPtr self = this;
    _router->addProxiesAsync(proxies,
        [self, cookie](const Ice::ObjectProxySeq& p)
        {
            self->addProxyResponse(p, cookie);
        },
        [self, cookie](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::Exception& ex)
            {
                self->addProxyException(ex, cookie);
            }
        });
#else
    _router->begin_addProxies(proxies,
                              newCallback_Router_addProxies(this,
                                                            &RouterInfo::addProxyResponse,
                                                            &RouterInfo::addProxyException),
                              cookie);
#endif
    return false;
}

void
IceInternal::RouterInfo::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Mutex::Lock sync(*this);
    _adapter = adapter;
}

ObjectAdapterPtr
IceInternal::RouterInfo::getAdapter() const
{
    IceUtil::Mutex::Lock sync(*this);
    return _adapter;
}

void
IceInternal::RouterInfo::clearCache(const ReferencePtr& ref)
{
    IceUtil::Mutex::Lock sync(*this);
    _identities.erase(ref->getIdentity());
}

vector<EndpointIPtr>
IceInternal::RouterInfo::setClientEndpoints(const Ice::ObjectPrxPtr& proxy, bool hasRoutingTable)
{
    IceUtil::Mutex::Lock sync(*this);
    if(_clientEndpoints.empty())
    {
        _hasRoutingTable = hasRoutingTable;
        if(!proxy)
        {
            //
            // If getClientProxy() return nil, use router endpoints.
            //
            _clientEndpoints = _router->_getReference()->getEndpoints();
        }
        else
        {
            Ice::ObjectPrxPtr clientProxy = proxy->ice_router(0); // The client proxy cannot be routed.

            //
            // In order to avoid creating a new connection to the router,
            // we must use the same timeout as the already existing
            // connection.
            //
            if(_router->ice_getConnection())
            {
                clientProxy = clientProxy->ice_timeout(_router->ice_getConnection()->timeout());
            }

            _clientEndpoints = clientProxy->_getReference()->getEndpoints();
        }
    }
    return _clientEndpoints;
}

void
IceInternal::RouterInfo::addAndEvictProxies(const Ice::ObjectPrxPtr& proxy, const Ice::ObjectProxySeq& evictedProxies)
{
    IceUtil::Mutex::Lock sync(*this);

    //
    // Check if the proxy hasn't already been evicted by a concurrent addProxies call.
    // If it's the case, don't add it to our local map.
    //
    multiset<Identity>::iterator p = _evictedIdentities.find(proxy->ice_getIdentity());
    if(p != _evictedIdentities.end())
    {
        _evictedIdentities.erase(p);
    }
    else
    {
        //
        // If we successfully added the proxy to the router,
        // we add it to our local map.
        //
        _identities.insert(proxy->ice_getIdentity());
    }

    //
    // We also must remove whatever proxies the router evicted.
    //
    for(Ice::ObjectProxySeq::const_iterator q = evictedProxies.begin(); q != evictedProxies.end(); ++q)
    {
        if(_identities.erase((*q)->ice_getIdentity()) == 0)
        {
            //
            // It's possible for the proxy to not have been
            // added yet in the local map if two threads
            // concurrently call addProxies.
            //
            _evictedIdentities.insert((*q)->ice_getIdentity());
        }
    }
}
