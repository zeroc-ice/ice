// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/SessionManager.h>

using namespace std;
using namespace IceGrid;

SessionManager::SessionManager(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
    if(communicator->getDefaultLocator())
    {
        Ice::ObjectPrx prx = communicator->getDefaultLocator();

        //
        // Derive the query objects from the locator proxy endpoints.
        //
        Ice::EndpointSeq endpoints = prx->ice_getEndpoints();
        Ice::Identity id = prx->ice_getIdentity();
        id.name = "Query";
        QueryPrx query = QueryPrx::uncheckedCast(prx->ice_identity(id));
        for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            Ice::EndpointSeq singleEndpoint;
            singleEndpoint.push_back(*p);
            _queryObjects.push_back(QueryPrx::uncheckedCast(query->ice_endpoints(singleEndpoint)));
        }

        id.name = "InternalRegistry-Master";
        _master = InternalRegistryPrx::uncheckedCast(prx->ice_identity(id)->ice_endpoints(Ice::EndpointSeq()));
    }
}

SessionManager::~SessionManager()
{
}

vector<QueryPrx>
SessionManager::findAllQueryObjects()
{
    vector<QueryPrx> queryObjects = _queryObjects;
    for(vector<QueryPrx>::const_iterator q = _queryObjects.begin(); q != _queryObjects.end(); ++q)
    {
        Ice::ConnectionPtr connection = (*q)->ice_getCachedConnection();
        if(connection)
        {
            try
            {
                connection->close(false);
            }
            catch(const Ice::LocalException&)
            {
            }
        }
    }

    map<Ice::Identity, QueryPrx> proxies;
    vector<Ice::AsyncResultPtr> results;
    size_t previousSize = 0;
    do
    {
        for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
        {
            results.push_back((*q)->begin_findAllObjectsByType(Registry::ice_staticId()));
        }

        map<Ice::Identity, QueryPrx> proxies;
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
                    Ice::Identity id = (*q)->ice_getIdentity();
                    id.name = "Query";
                    proxies[(*q)->ice_getIdentity()] = QueryPrx::uncheckedCast((*q)->ice_identity(id));
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
            queryObjects.push_back(p->second);
        }
    }
    while(proxies.size() != previousSize);
    return queryObjects;
}
