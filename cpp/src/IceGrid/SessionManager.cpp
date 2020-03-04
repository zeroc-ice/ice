//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/SessionManager.h>

#include <set>

using namespace std;
using namespace IceGrid;

SessionManager::SessionManager(const shared_ptr<Ice::Communicator>& communicator, const string& instanceName) :
    _communicator(communicator), _instanceName(instanceName)
{
    auto prx = communicator->getDefaultLocator();
    if(prx)
    {
        Ice::Identity id = { "InternalRegistry-Master", instanceName };
        _master =
            Ice::uncheckedCast<InternalRegistryPrx>(prx->ice_identity(move(id))->ice_endpoints({}));
    }
}

vector<shared_ptr<QueryPrx>>
SessionManager::findAllQueryObjects(bool cached)
{
    vector<shared_ptr<QueryPrx>> queryObjects;
    shared_ptr<Ice::LocatorPrx> locator;
    {
        lock_guard lock(_mutex);
        if(!_communicator)
        {
            return queryObjects;
        }
        if(cached && !_queryObjects.empty())
        {
            return _queryObjects;
        }
        queryObjects = _queryObjects;
        locator = _communicator->getDefaultLocator();
    }

    if(!cached)
    {
        for(const auto& queryObject : queryObjects)
        {
            auto connection = queryObject->ice_getCachedConnection();
            if(connection)
            {
                try
                {
                    connection->close(Ice::ConnectionClose::GracefullyWithWait);
                }
                catch(const Ice::LocalException&)
                {
                }
            }
        }
        queryObjects.clear();
    }

    if(queryObjects.empty() && locator)
    {
        Ice::Identity id = { "Query", _instanceName };
        auto query = Ice::uncheckedCast<QueryPrx>(locator->ice_identity(id));
        auto endpoints = query->ice_getEndpoints();
        if(endpoints.empty())
        {
            try
            {
                auto r = locator->findObjectById(id);
                if(r)
                {
                    endpoints = r->ice_getEndpoints();
                }
            }
            catch(const Ice::Exception&)
            {
                // Ignore.
            }
        }

        for(const auto& endpoint : endpoints)
        {
            queryObjects.push_back(Ice::uncheckedCast<QueryPrx>(query->ice_endpoints({endpoint})));
        }
    }

    //
    // Find all known query objects by querying all the registries we can find.
    //
    map<Ice::Identity, shared_ptr<QueryPrx>> proxies;
    set<shared_ptr<QueryPrx>> requested;
    while(true)
    {
        vector<future<Ice::ObjectProxySeq>> results;
        for(const auto& queryObject : queryObjects)
        {
            results.push_back(queryObject->findAllObjectsByTypeAsync(Registry::ice_staticId()));
            requested.insert(queryObject);
        }
        if(results.empty())
        {
            break;
        }

        for(auto& result : results)
        {
            if(isDestroyed())
            {
                break;
            }

            try
            {
                auto prxs = result.get();
                for(const auto& prx : prxs)
                {
                    if(proxies.find(prx->ice_getIdentity()) == proxies.end())
                    {
                        //
                        // Add query proxy for each IceGrid registry. The proxy contains the endpoints
                        // of the registry since it's based on the registry interface proxy.
                        //
                        Ice::Identity id = { "Query", prx->ice_getIdentity().category };
                        proxies[prx->ice_getIdentity()] = Ice::uncheckedCast<QueryPrx>(prx->ice_identity(move(id)));
                    }
                }
            }
            catch(const Ice::Exception&)
            {
                // Ignore.
            }
        }

        queryObjects.clear();
        for(const auto& prx : proxies)
        {
            if(requested.find(prx.second) == requested.end())
            {
                queryObjects.push_back(prx.second);
            }
        }
    }

    lock_guard lock(_mutex);
    _queryObjects.clear();
    for(const auto& prx : proxies)
    {
        _queryObjects.push_back(prx.second);
    }
    return _queryObjects;
}
