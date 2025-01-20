// Copyright (c) ZeroC, Inc.

#include "SessionManager.h"
#include "Ice/Ice.h"

#include <set>

using namespace std;
using namespace IceGrid;

SessionManager::SessionManager(const shared_ptr<Ice::Communicator>& communicator, const string& instanceName)
    : _communicator(communicator),
      _instanceName(instanceName)
{
    optional<Ice::LocatorPrx> defaultLocator = communicator->getDefaultLocator();
    if (defaultLocator)
    {
        _master =
            defaultLocator->ice_identity<InternalRegistryPrx>(Ice::Identity{"InternalRegistry-Master", instanceName})
                ->ice_endpoints({});
    }
}

vector<QueryPrx>
SessionManager::findAllQueryObjects(bool cached)
{
    vector<QueryPrx> queryObjects;
    optional<Ice::LocatorPrx> locator;
    {
        lock_guard lock(_mutex);
        if (!_communicator)
        {
            return queryObjects;
        }
        if (cached && !_queryObjects.empty())
        {
            return _queryObjects;
        }
        queryObjects = _queryObjects;
        locator = _communicator->getDefaultLocator();
    }

    if (!cached)
    {
        for (const auto& queryObject : queryObjects)
        {
            auto connection = queryObject->ice_getCachedConnection();
            if (connection)
            {
                try
                {
                    connection->close().get();
                }
                catch (...)
                {
                }
            }
        }
        queryObjects.clear();
    }

    if (queryObjects.empty() && locator)
    {
        Ice::Identity id{"Query", _instanceName};
        auto query = (*locator)->ice_identity<QueryPrx>(id);
        auto endpoints = query->ice_getEndpoints();
        if (endpoints.empty())
        {
            try
            {
                auto r = locator->findObjectById(id);
                if (r)
                {
                    endpoints = r->ice_getEndpoints();
                }
            }
            catch (const Ice::Exception&)
            {
                // Ignore.
            }
        }

        for (const auto& endpoint : endpoints)
        {
            queryObjects.push_back(query->ice_endpoints({endpoint}));
        }
    }

    // Find all known query objects by querying all the registries we can find.
    map<Ice::Identity, QueryPrx> proxies;
    set<QueryPrx> requested;
    while (true)
    {
        vector<future<Ice::ObjectProxySeq>> results;
        for (const auto& queryObject : queryObjects)
        {
            results.push_back(queryObject->findAllObjectsByTypeAsync(Registry::ice_staticId()));
            requested.insert(queryObject);
        }
        if (results.empty())
        {
            break;
        }

        for (auto& result : results)
        {
            if (isDestroyed())
            {
                break;
            }

            try
            {
                for (const auto& prx : result.get())
                {
                    assert(prx);
                    if (proxies.find(prx->ice_getIdentity()) == proxies.end())
                    {
                        // Add query proxy for each IceGrid registry. The proxy contains the endpoints
                        // of the registry since it's based on the registry interface proxy.
                        proxies.insert(
                            {prx->ice_getIdentity(),
                             (*prx)->ice_identity<QueryPrx>(Ice::Identity{"Query", prx->ice_getIdentity().category})});
                    }
                }
            }
            catch (const Ice::Exception&)
            {
                // Ignore.
            }
        }

        queryObjects.clear();
        for (const auto& prx : proxies)
        {
            if (requested.find(prx.second) == requested.end())
            {
                queryObjects.push_back(prx.second);
            }
        }
    }

    lock_guard lock(_mutex);
    _queryObjects.clear();
    for (const auto& prx : proxies)
    {
        _queryObjects.push_back(prx.second);
    }
    return _queryObjects;
}
