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
#include <Glacier/ServerBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ServerBlobject::ServerBlobject(const ObjectAdapterPtr& clientAdapter) :
    _clientAdapter(clientAdapter),
    _logger(_clientAdapter->getCommunicator()->getLogger())
{
    PropertiesPtr properties = _clientAdapter->getCommunicator()->getProperties();
    _traceLevel = atoi(properties->getProperty("Glacier.Trace.Server").c_str());
}

Glacier::ServerBlobject::~ServerBlobject()
{
    assert(!_clientAdapter);
}

void
Glacier::ServerBlobject::destroy()
{
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    _clientAdapter = 0;
    _logger = 0;
}

void
Glacier::ServerBlobject::ice_invoke(const std::vector<Byte>& inParams, std::vector<Byte>& outParams,
				    const Current& current)
{
    assert(_clientAdapter); // Destroyed?

    try
    {
	ObjectPrx proxy = _clientAdapter->createReverseProxy(current.identity);
	assert(proxy);

	if (!current.facet.empty())
	{
	    proxy = proxy->ice_newFacet(current.facet);
	}

	if (!current.response)
	{
	    proxy = proxy->ice_oneway();
	}
	
	if (_traceLevel >= 2)
	{
	    ostringstream s;
	    s << "reverse routing to:\n"
	      << "proxy = " << _clientAdapter->getCommunicator()->proxyToString(proxy) << '\n'
	      << "operation = " << current.operation << '\n'
	      << "nonmutating = " << (current.nonmutating ? "true" : "false");
	    _logger->trace("Glacier", s.str());
	}

	proxy->ice_invoke(current.operation, current.nonmutating, inParams, outParams, current.context);
    }
    catch (const Exception& ex)
    {
	if (_traceLevel)
	{
	    ostringstream s;
	    s << "reverse routing exception:\n" << ex;
	    _logger->trace("Glacier", s.str());
	}

	ex.ice_throw();
    }
}
