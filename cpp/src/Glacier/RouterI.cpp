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
    _logger(_clientAdapter->getCommunicator()->getLogger()),
    _routingTable(routingTable)
{
    PropertiesPtr properties = _clientAdapter->getCommunicator()->getProperties();
    _routingTableTraceLevel = atoi(properties->getProperty("Glacier.Trace.RoutingTable").c_str());
}

Glacier::RouterI::~RouterI()
{
    assert(!_clientAdapter == 0);
}

void
Glacier::RouterI::destroy()
{
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    _clientAdapter = 0;
    _serverAdapter = 0;
    _logger = 0;
    _routingTable = 0;
}

ObjectPrx
Glacier::RouterI::getClientProxy(const Current&)
{
    assert(_clientAdapter);
    return _clientAdapter->createProxy(stringToIdentity("dummy"));
}

ObjectPrx
Glacier::RouterI::getServerProxy(const Current&)
{
    if (_serverAdapter)
    {
	return _serverAdapter->createProxy(stringToIdentity("dummy"));
    }
    else
    {
	return 0;
    }
}

void
Glacier::RouterI::addProxy(const ObjectPrx& proxy, const Current&)
{
    if (_routingTableTraceLevel)
    {
	ostringstream s;
	s << "adding proxy to routing table:\n"
	  << _clientAdapter->getCommunicator()->proxyToString(proxy);
	_logger->trace("Glacier", s.str());
    }

    _routingTable->add(proxy);
}
