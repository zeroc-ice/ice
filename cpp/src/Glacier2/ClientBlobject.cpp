// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Ice/IdentityUtil.h>
#include <Glacier2/ClientBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					 const IceInternal::RoutingTablePtr& routingTable,
					 const StringSeq& allowCategories) :
    Glacier2::Blobject(communicator, false),
    _routingTable(routingTable),
    _allowCategories(allowCategories),
    _rejectTraceLevel(_properties->getPropertyAsInt("Glacier2.Client.Trace.Reject"))
{
}

Glacier2::ClientBlobject::~ClientBlobject()
{
    assert(!_routingTable);
}

void
Glacier2::ClientBlobject::destroy()
{
    assert(_routingTable); // Destroyed?
    _routingTable = 0;
    Blobject::destroy();
}

void
Glacier2::ClientBlobject::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB, const ByteSeq& inParams,
					   const Current& current)
{
    assert(_routingTable); // Destroyed?

    //
    // If there is an _allowCategories set then enforce it.
    //
    if(!_allowCategories.empty())
    {
	if(!binary_search(_allowCategories.begin(), _allowCategories.end(), current.id.category))
	{
	    if(_rejectTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "rejecting request\n";
		out << "identity: " << identityToString(current.id);
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
	ex.id = current.id;
	throw ex;
    }

    invoke(proxy, amdCB, inParams, current);
}
