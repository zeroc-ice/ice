// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ClientBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					 const RoutingTablePtr& routingTable,
					 const StringSeq& allowCategories) :
    Glacier2::Blobject(communicator, false),
    _routingTable(routingTable),
    _allowCategories(allowCategories),
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
    //
    // If there is an _allowCategories set then enforce it.
    //
    if(!_allowCategories.empty())
    {
	//
	// Note: Some filtering will be relying on regular
	// expressions. A quick performance test revealed that on Linux
	// using the standard regular expression API to create an
	// equivalent mechanism was much slower when there were a large
	// number of categories. It would appear that the simple
	// mechanism used here is a better choice, at least on Linux.
	//
	if(!binary_search(_allowCategories.begin(), _allowCategories.end(), current.id.category))
	{
	    if(_rejectTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "rejecting request\n";
		out << "identity: " << _communicator->identityToString(current.id);
	    }
	    ObjectNotExistException ex(__FILE__, __LINE__);
	    ex.id = current.id;
	    throw ex;
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
	//ex.operation = current.operation;
	ex.operation = "ice_add_proxy";
	throw ex;
    }

    invoke(proxy, amdCB, inParams, current);
}
