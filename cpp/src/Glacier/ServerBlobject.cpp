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
    _traceLevel = properties->getPropertyAsInt("Glacier.Router.Trace.Server");
}

Glacier::ServerBlobject::~ServerBlobject()
{
    assert(!_clientAdapter);
}

void
Glacier::ServerBlobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _clientAdapter = 0;
    _logger = 0;
}

bool
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
			Warning out(_logger);
			out << "unknown forward option `" << option << "'";
			break;
		    }
		}
	    }
	}
	
	if (_traceLevel >= 2)
	{
	    Trace out(_logger, "Glacier");
	    out << "reverse routing to:\n"
		<< "proxy = " << _clientAdapter->getCommunicator()->proxyToString(proxy) << '\n'
		<< "operation = " << current.operation << '\n'
		<< "nonmutating = " << (current.nonmutating ? "true" : "false");
	}

	return proxy->ice_invoke(current.operation, current.nonmutating, inParams, outParams, current.context);
    }
    catch (const Exception& ex)
    {
	if (_traceLevel)
	{
	    Trace out(_logger, "Glacier");
	    out << "reverse routing exception:\n" << ex;
	}

	ex.ice_throw();
    }

    assert(false);
    return true; // To keep the compiler happy.
}
