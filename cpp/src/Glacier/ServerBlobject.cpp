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
    Glacier::Blobject(_clientAdapter->getCommunicator()),
    _clientAdapter(clientAdapter)
{
    PropertiesPtr properties = _communicator->getProperties();
    _traceLevel = properties->getPropertyAsInt("Glacier.Router.Trace.Server");
}

void
Glacier::ServerBlobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _clientAdapter = 0;
    Glacier::Blobject::destroy();
}

bool
Glacier::ServerBlobject::ice_invoke(const vector<Byte>& inParams, vector<Byte>& outParams, const Current& current)
{
    assert(_clientAdapter); // Destroyed?

    try
    {
	ObjectPrx proxy = _clientAdapter->createReverseProxy(current.identity);
	assert(proxy);

	MissiveQueuePtr missiveQueue = modifyProxy(proxy, current);
	assert(!missiveQueue);
	
	if (_traceLevel >= 2)
	{
	    Trace out(_logger, "Glacier");
	    out << "reverse routing to:\n"
		<< "proxy = " << _communicator->proxyToString(proxy) << '\n'
		<< "operation = " << current.operation << '\n'
		<< "nonmutating = " << (current.nonmutating ? "true" : "false");
	}

	// TODO: Should we forward the context? Perhaps a config parameter?
	return proxy->ice_invoke(current.operation, current.nonmutating, inParams, outParams, current.context);
    }
    catch (const Exception& ex)
    {
	if (_traceLevel >= 1)
	{
	    Trace out(_logger, "Glacier");
	    out << "reverse routing exception:\n" << ex;
	}

	ex.ice_throw();
    }

    assert(false);
    return true; // To keep the compiler happy.
}
