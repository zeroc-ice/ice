// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TransportInfoI.h>
#include <Ice/ConnectionI.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::TransportInfoI::flushBatchRequests()
{
    ConnectionIPtr connection;

    {
	IceUtil::Mutex::Lock sync(_connectionMutex);
	connection = _connection;
    }

    //
    // We flush outside the mutex lock, to avoid potential deadlocks.
    //
    if(connection)
    {
	connection->flushBatchRequest();
    }
}

ObjectPrx
Ice::TransportInfoI::createProxy(const Identity& ident) const
{
    IceUtil::Mutex::Lock sync(_connectionMutex);

    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    vector<EndpointPtr> endpoints;
    vector<ConnectionIPtr> connections;
    connections.push_back(_connection);
    ReferencePtr ref = _connection->instance()->referenceFactory()->create(ident, Context(), "", Reference::ModeTwoway,
									   false, "", endpoints, 0, 0,
									   connections, true);
    return _connection->instance()->proxyFactory()->referenceToProxy(ref);
}

void
Ice::TransportInfoI::setConnection(const ConnectionIPtr& connection)
{
    IceUtil::Mutex::Lock sync(_connectionMutex);
    _connection = connection;
}
