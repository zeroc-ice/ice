
// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
using namespace Glacier;

Glacier::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					const IceInternal::RoutingTablePtr& routingTable,
					const string& allowCategories) :
    Glacier::Blobject(communicator, false),
    _routingTable(routingTable)
{
    const string ws = " \t";
    string::size_type current = allowCategories.find_first_not_of(ws, 0);
    while(current != string::npos)
    {
	string::size_type pos = allowCategories.find_first_of(ws, current);
	string::size_type len = (pos == string::npos) ? string::npos : pos - current;
	string category = allowCategories.substr(current, len);
	_allowCategories.push_back(category);
	current = allowCategories.find_first_not_of(ws, pos);
    }
    sort(_allowCategories.begin(), _allowCategories.end()); // Must be sorted.
    _allowCategories.erase(unique(_allowCategories.begin(), _allowCategories.end()), _allowCategories.end());
}

void
Glacier::ClientBlobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _routingTable = 0;
    Blobject::destroy();
}

void
Glacier::ClientBlobject::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB, const vector<Byte>& inParams,
					  const Current& current)
{
    assert(_communicator); // Destroyed?

    //
    // If there is an _allowCategories set then enforce it.
    //
    if(!_allowCategories.empty())
    {
	if(!binary_search(_allowCategories.begin(), _allowCategories.end(), current.id.category))
	{
	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
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
