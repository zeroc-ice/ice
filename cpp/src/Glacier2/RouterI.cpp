// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier2/RouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RouterI::RouterI(const ObjectAdapterPtr& clientAdapter,
			  const ObjectAdapterPtr& serverAdapter,
			  const IceInternal::RoutingTablePtr& routingTable) :
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _logger(_clientAdapter->getCommunicator()->getLogger()),
    _routingTable(routingTable),
    _userId("todo")
{
    CommunicatorPtr communicator = _clientAdapter->getCommunicator();
    PropertiesPtr properties = communicator->getProperties();

    _routingTableTraceLevel = properties->getPropertyAsInt("Glacier2.Trace.RoutingTable");
}

Glacier2::RouterI::~RouterI()
{
    assert(!_clientAdapter);
}

void
Glacier2::RouterI::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _clientAdapter = 0;
    _serverAdapter = 0;
    _logger = 0;
    _routingTable = 0;
}

ObjectPrx
Glacier2::RouterI::getClientProxy(const Current&) const
{
    assert(_clientAdapter); // Destroyed?

    return _clientAdapter->createProxy(stringToIdentity("dummy"));
}

ObjectPrx
Glacier2::RouterI::getServerProxy(const Current&) const
{
    assert(_clientAdapter); // Destroyed?

    if(_serverAdapter)
    {
	return _serverAdapter->createProxy(stringToIdentity("dummy"));
    }
    else
    {
	return 0;
    }
}

void
Glacier2::RouterI::addProxy(const ObjectPrx& proxy, const Current&)
{
    assert(_clientAdapter); // Destroyed?

    if(_routingTableTraceLevel)
    {
	Trace out(_logger, "Glacier2");
	out << "adding proxy to routing table:\n" << _clientAdapter->getCommunicator()->proxyToString(proxy);
    }

    _routingTable->add(proxy);
}

void
Glacier2::RouterI::createSession(const std::string&, const std::string&, const Ice::Current&)
{
}
