// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier/RouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::RouterI::RouterI(const ObjectAdapterPtr& clientAdapter,
			  const ObjectAdapterPtr& serverAdapter,
			  const ::IceInternal::RoutingTablePtr& routingTable) :
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _routingTable(routingTable)
{
}

ObjectPrx
Glacier::RouterI::getClientProxy(const Current&)
{
    return _clientAdapter->createProxy(stringToIdentity("dummy"));
}

ObjectPrx
Glacier::RouterI::getServerProxy(const Current&)
{
    return _serverAdapter->createProxy(stringToIdentity("dummy"));
}

void
Glacier::RouterI::addProxy(const ObjectPrx& proxy, const Current&)
{
    _routingTable->add(proxy);
}
