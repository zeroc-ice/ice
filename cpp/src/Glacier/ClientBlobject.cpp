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

Glacier::ClientBlobject::~ClientBlobject()
{
    assert(!_communicator);
}

void
Glacier::ClientBlobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _communicator = 0;
    _logger = 0;
    _routingTable = 0;
}

bool
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

	Context::const_iterator p = current.context.find("_fwd");
	if (p != current.context.end())
	{
	    for (unsigned int i = 0; i < p->second.length(); ++i)
	    {
		char option = p->second[i];
		switch (option)
		{
		    case 't':
		    {
			proxy = proxy->ice_twoway();
			break;
		    }
		    
		    case 'o':
		    {
			proxy = proxy->ice_oneway();
			break;
		    }
		    
		    case 'd':
		    {
			proxy = proxy->ice_datagram();
			break;
		    }
		    
		    case 's':
		    {
			proxy = proxy->ice_secure(true);
			break;
		    }
		    
		    default:
		    {
			ostringstream s;
			s << "unknown forward option `" << option << "'";
			_logger->warning(s.str());
			break;
		    }
		}
	    }
	}
	
	if (_traceLevel >= 2)
	{
	    ostringstream s;
	    s << "routing to:\n"
	      << "proxy = " << _communicator->proxyToString(proxy) << '\n'
	      << "operation = " << current.operation << '\n'
	      << "nonmutating = " << (current.nonmutating ? "true" : "false");
	    _logger->trace("Glacier", s.str());
	}

	return proxy->ice_invoke(current.operation, current.nonmutating, inParams, outParams, current.context);
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

    assert(false);
    return true; // To keep the compiler happy.
}
