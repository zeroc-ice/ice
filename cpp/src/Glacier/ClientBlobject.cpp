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
#include <Ice/IdentityUtil.h>

#include <Glacier/ClientBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					const IceInternal::RoutingTablePtr& routingTable,
					const string& allowCategories) :
    Glacier::Blobject(communicator),
    _routingTable(routingTable)
{
    PropertiesPtr properties = _communicator->getProperties();
    _traceLevel = properties->getPropertyAsInt("Glacier.Router.Trace.Client");
    _forwardContext = properties->getPropertyAsInt("Glacier.Router.Client.ForwardContext") > 0;
    _batchSleepTime = IceUtil::Time::milliSeconds(
	properties->getPropertyAsIntWithDefault("Glacier.Router.Client.BatchSleepTime", 250));

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

bool
Glacier::ClientBlobject::reverse()
{
    return false;
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

bool
Glacier::ClientBlobject::ice_invoke(const vector<Byte>& inParams, vector<Byte>& outParams, const Current& current)
{
    assert(_communicator); // Destroyed?

    //
    // If there is an _allowCategories set then enforce it.
    //
    if(!_allowCategories.empty())
    {
	if(!binary_search(_allowCategories.begin(), _allowCategories.end(), current.identity.category))
	{
	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
		out << "rejecting request\n";
		out << "identity: " << identityToString(current.identity);
	    }
	    ObjectNotExistException ex(__FILE__, __LINE__);
	    ex.identity = current.identity;
	    throw ex;
	}
    }

    ObjectPrx proxy = _routingTable->get(current.identity);
    if(!proxy)
    {
	ObjectNotExistException ex(__FILE__, __LINE__);
	ex.identity = current.identity;
	throw ex;
    }

    return invoke(proxy, inParams, outParams, current);
}
