// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ClientBlobject.h>
#include <Glacier2/FilterManager.h>
#include <Glacier2/FilterI.h>
#include <Glacier2/RoutingTable.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					 const FilterManagerPtr& filters):
					 
    Glacier2::Blobject(communicator, false),
    _routingTable(new RoutingTable(communicator)),
    _filters(filters),
    _rejectTraceLevel(_properties->getPropertyAsInt("Glacier2.Client.Trace.Reject"))
{
}

Glacier2::ClientBlobject::~ClientBlobject()
{
}

void
Glacier2::ClientBlobject::ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr& amdCB, 
					   const std::pair<const Byte*, const Byte*>& inParams,
					   const Current& current)
{
    bool rejected = false;
 
    if(!_filters->categories()->empty())
    {
	if(!_filters->categories()->match(current.id.category))
	{
	    if(_rejectTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "rejecting request\n";
		out << "identity: " << _communicator->identityToString(current.id);
	    }
	    rejected = true;
	}
    }

    if(!_filters->identities()->empty())
    {
	if(_filters->identities()->match(current.id))
	{
	    rejected = false;
	}
	else
	{
	    if(_rejectTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "rejecting request\n";
		out << "identity: " << _communicator->identityToString(current.id);
	    }
	    rejected = true;
	}
    }

    ObjectPrx proxy = _routingTable->get(current.id);
    if(!proxy)
    {
	ObjectNotExistException ex(__FILE__, __LINE__);

	//
	// We use a special operation name indicate to the client that
	// the proxy for the Ice object has not been found in our
	// routing table. This can happen if the proxy was evicted
	// from the routing table.
	//
	ex.id = current.id;
	ex.facet = current.facet;
	ex.operation = "ice_add_proxy";
	throw ex;
    }

    string adapterId = proxy->ice_getAdapterId();

    if(!adapterId.empty() && !_filters->adapterIds()->empty())
    {
	if(_filters->adapterIds()->match(adapterId))
	{
	    rejected  = false;
	}
	else
	{
	    if(_rejectTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "rejecting request\n";
		out << "identity: " << _communicator->identityToString(current.id);
	    }
	}
    }

    if(rejected)
    {
	ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }
    invoke(proxy, amdCB, inParams, current);
}

ObjectProxySeq
Glacier2::ClientBlobject::add(const ObjectProxySeq& proxies, const Current& current)
{
    return _routingTable->add(proxies, current);
}

StringSetPtr 
ClientBlobject::categories()
{
    return _filters->categories();
}

StringSetPtr 
ClientBlobject::adapterIds()
{
    return _filters->adapterIds();
}

IdentitySetPtr
ClientBlobject::identities()
{
    return _filters->identities();
}
