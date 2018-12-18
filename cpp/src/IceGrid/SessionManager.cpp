// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/SessionManager.h>

#include <set>

using namespace std;
using namespace IceGrid;

SessionManager::SessionManager(const Ice::CommunicatorPtr& communicator, const string& instanceName) :
    _communicator(communicator), _instanceName(instanceName)
{
    Ice::LocatorPrx prx = communicator->getDefaultLocator();
    if(prx)
    {
        Ice::Identity id;
        id.category = instanceName;
        id.name = "InternalRegistry-Master";
        _master = InternalRegistryPrx::uncheckedCast(prx->ice_identity(id)->ice_endpoints(Ice::EndpointSeq()));
    }
}

SessionManager::~SessionManager()
{
}

vector<QueryPrx>
SessionManager::findAllQueryObjects(bool cached)
{
    vector<QueryPrx> queryObjects;
    Ice::LocatorPrx locator;
    {
        Lock sync(*this);
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
        for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
        {
            Ice::ConnectionPtr connection = (*q)->ice_getCachedConnection();
            if(connection)
            {
                try
                {
                    connection->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
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
        Ice::Identity id;
        id.category = _instanceName;
        id.name = "Query";
        QueryPrx query = QueryPrx::uncheckedCast(locator->ice_identity(id));
        Ice::EndpointSeq endpoints = query->ice_getEndpoints();
        if(endpoints.empty())
        {
            try
            {
                Ice::ObjectPrx r = locator->findObjectById(id);
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

        for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            Ice::EndpointSeq singleEndpoint;
            singleEndpoint.push_back(*p);
            queryObjects.push_back(QueryPrx::uncheckedCast(query->ice_endpoints(singleEndpoint)));
        }
    }

    //
    // Find all known query objects by querying all the registries we can find.
    //
    map<Ice::Identity, QueryPrx> proxies;
    set<QueryPrx> requested;
    while(true)
    {
        vector<Ice::AsyncResultPtr> results;
        for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
        {
            results.push_back((*q)->begin_findAllObjectsByType(Registry::ice_staticId()));
            requested.insert(*q);
        }
        if(results.empty())
        {
            break;
        }

        for(vector<Ice::AsyncResultPtr>::const_iterator p = results.begin(); p != results.end(); ++p)
        {
            QueryPrx query = QueryPrx::uncheckedCast((*p)->getProxy());
            if(isDestroyed())
            {
                break;
            }

            try
            {
                Ice::ObjectProxySeq prxs = query->end_findAllObjectsByType(*p);
                for(Ice::ObjectProxySeq::iterator q = prxs.begin(); q != prxs.end(); ++q)
                {
                    if(proxies.find((*q)->ice_getIdentity()) == proxies.end())
                    {
                        //
                        // Add query proxy for each IceGrid registry. The proxy contains the endpoints
                        // of the registry since it's based on the registry interface proxy.
                        //
                        Ice::Identity id = (*q)->ice_getIdentity();
                        id.name = "Query";
                        proxies[(*q)->ice_getIdentity()] = QueryPrx::uncheckedCast((*q)->ice_identity(id));
                    }
                }
            }
            catch(const Ice::Exception&)
            {
                // Ignore.
            }
        }

        queryObjects.clear();
        for(map<Ice::Identity, QueryPrx>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
        {
            if(requested.find(p->second) == requested.end())
            {
                queryObjects.push_back(p->second);
            }
        }
    }

    Lock sync(*this);
    _queryObjects.clear();
    for(map<Ice::Identity, QueryPrx>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        _queryObjects.push_back(p->second);
    }
    return _queryObjects;
}
