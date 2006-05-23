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
					 const StringFilterIPtr& categoryFilter,
					 const StringFilterIPtr& adapterIdFilter,
					 const IdentityFilterIPtr& objectIdFilter): 
    Glacier2::Blobject(communicator, false),
    _routingTable(routingTable),
    _categoryFilter(categoryFilter),
    _adapterIdFilter(adapterIdFilter),
    _objectIdFilter(objectIdFilter),
    _filtersEnabled((categoryFilter && adapterIdFilter && objectIdFilter)),
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
    // The filter matching could all be grouped together after the routing
    // table search. However, in the event that this is going to be
    // filtered out it might be better to avoid the lookup. We have to do
    // the adapter id match after the lookup because we don't know what the
    // adapter id is until we lookup the proxy.
    //
    if(_filtersEnabled)
    {
	if(!_categoryFilter->match(current.id.category))
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

	if(!_objectIdFilter->match(current.id))
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

    if(_filtersEnabled)
    {
	string adapterId = proxy->ice_getAdapterId();
	if(!adapterId.empty() && !_adapterIdFilter->match(adapterId))
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
    invoke(proxy, amdCB, inParams, current);
}
