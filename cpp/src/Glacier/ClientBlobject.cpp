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
#include <Glacier/ClientBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					const IceInternal::RoutingTablePtr& routingTable) :
    _communicator(communicator),
    _logger(_communicator->getLogger()),
    _routingTable(routingTable)
{
    PropertiesPtr properties = _communicator->getProperties();
    _traceLevel = atoi(properties->getProperty("Glacier.Trace.Client").c_str());
}

void
Glacier::ClientBlobject::destroy()
{
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    _communicator = 0;
    _logger = 0;
    _routingTable = 0;
}

void
Glacier::ClientBlobject::ice_invoke(const std::vector<Byte>& inParams, std::vector<Byte>& outParams,
				    const Current& current)
{
    assert(_communicator); // Destroyed?

    try
    {
	ObjectPrx proxy = _routingTable->get(current.identity);
	
	if (!proxy)
	{
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
	
	if (!current.facet.empty())
	{
	    proxy = proxy->ice_newFacet(current.facet);
	}
	
	if (!current.response)
	{
	    proxy = proxy->ice_oneway();
	}
	
	proxy->ice_invoke(current.operation, current.nonmutating, inParams, outParams, current.context);
    }
    catch (const Exception& ex)
    {
	if (_traceLevel)
	{
	    ostringstream s;
	    s << "routing exception:\n" << ex;
	    _logger->trace("Glacier", s.str());
	}

	ex.ice_throw();
    }
}
