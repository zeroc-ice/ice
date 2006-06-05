// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ClientBlobject.h>
#include <Glacier2/FilterI.h>
#include <Glacier2/RoutingTable.h>

using namespace std;
using namespace Ice;

namespace Glacier2
{

class ClientBlobjectImpl
{
public:

    ClientBlobjectImpl(const StringFilterManagerIPtr& categoryFilter, const StringFilterManagerIPtr& adapterFilter,
		       const IdentityFilterManagerIPtr& idFilter) :
	_categoryFilter(categoryFilter),
	_adapterIdFilter(adapterFilter),
	_identityFilter(idFilter)
    {
    }

    StringFilterManagerIPtr 
    categoryFilter()
    {
	return _categoryFilter;
    }

    StringFilterManagerIPtr 
    adapterIdFilter()
    {
	return _adapterIdFilter;
    }

    IdentityFilterManagerIPtr 
    identityFilter()
    {
	return _identityFilter;
    }

private:
    const StringFilterManagerIPtr _categoryFilter;
    const StringFilterManagerIPtr _adapterIdFilter;
    const IdentityFilterManagerIPtr _identityFilter;
};

}

//
// Parse a space delimited string into a sequence of strings.
//
static void
stringToSeq(const string& str, vector<string>& seq)
{
    string const ws = " \t";
    string::size_type current = str.find_first_not_of(ws, 0);
    while(current != string::npos)
    {
	string::size_type pos = str.find_first_of(ws, current);
	string::size_type len = (pos == string::npos) ? string::npos : pos - current;
	seq.push_back(str.substr(current, len));
	current = str.find_first_not_of(ws, pos);
    }
}

using namespace Glacier2;

//
// Parse a space delimited string into a sequence of identities.
// XXX- this isn't correct, strictly speaking. We need a way to specify
// and parse object identities in a configuration file
//
static void
stringToSeq(const CommunicatorPtr& comm, const string& str, vector<Identity>& seq)
{
    string const ws = " \t";
    string::size_type current = str.find_first_not_of(ws, 0);
    while(current != string::npos)
    {
	string::size_type pos = str.find_first_of(ws, current);
	string::size_type len = (pos == string::npos) ? string::npos : pos - current;
	seq.push_back(comm->stringToIdentity(str.substr(current, len)));
	current = str.find_first_not_of(ws, pos);
    }
}

Glacier2::ClientBlobject::ClientBlobject(const CommunicatorPtr& communicator,
					 ClientBlobjectImpl* impl):
					 
    Glacier2::Blobject(communicator, false),
    _routingTable(new RoutingTable(communicator)),
    _impl(impl),
    _rejectTraceLevel(_properties->getPropertyAsInt("Glacier2.Client.Trace.Reject"))
{
}

Glacier2::ClientBlobject::~ClientBlobject()
{
    delete _impl;
}

void
Glacier2::ClientBlobject::ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr& amdCB, 
					   const std::pair<const Byte*, const Byte*>& inParams,
					   const Current& current)
{
    if(!_impl->categoryFilter()->match(current.id.category))
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

    if(!_impl->identityFilter()->match(current.id))
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

    string adapterId = proxy->ice_getAdapterId();
    if(!adapterId.empty() && !_impl->adapterIdFilter()->match(adapterId))
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
    invoke(proxy, amdCB, inParams, current);
}

ObjectProxySeq
Glacier2::ClientBlobject::add(const ObjectProxySeq& proxies, const Current& current)
{
    return _routingTable->add(proxies, current);
}

ClientBlobjectPtr
Glacier2::ClientBlobject::create(const CommunicatorPtr& communicator, const string& userId)
{
    PropertiesPtr props = communicator->getProperties();
    //
    // DEPRECATED PROPERTY: Glacier2.AllowCategories is to be deprecated
    // and superseded by Glacier2.Filter.Categories.Allow.
    //
    string allow = props->getProperty("Glacier2.AllowCategories");
    if(allow.empty())
    {
	allow = props->getProperty("Glacier2.Filter.Category.Accept");
    }

    string reject = props->getProperty("Glacier2.Filter.Category.Reject");
    bool acceptOverride = props->getPropertyAsIntWithDefault("Glacier2.Filter.Category.AcceptOverride", 0) == 1;

    vector<string> allowSeq;
    vector<string> rejectSeq;
    stringToSeq(allow, allowSeq);
    stringToSeq(reject, rejectSeq);

    int addUserMode = props->getPropertyAsInt("Glacier2.AddUserToAllowCategories");
    if(addUserMode == 0)
    {
	addUserMode = props->getPropertyAsInt("Glacier2.Filter.Category.AddUser");
    }
   
    if(addUserMode > 0 && !userId.empty())
    {
	if(addUserMode == 1)
	{
	    allowSeq.push_back(userId); // Add user id to allowed categories.
	}
	else if(addUserMode == 2)
	{
	    allowSeq.push_back('_' + userId); // Add user id with prepended underscore to allowed categories.
	}
    }	
    StringFilterManagerIPtr categoryFilter = new StringFilterManagerI(allowSeq, rejectSeq, acceptOverride);

    //
    // TODO: refactor initialization of filters.
    //
    allow = props->getProperty("Glacier2.Filter.AdapterId.Accept");
    reject = props->getProperty("Glacier2.Filter.AdapterId.Reject");
    acceptOverride = props->getPropertyAsIntWithDefault("Glacier2.Filter.AdapterId.AcceptOverride", 0) == 1;
    stringToSeq(allow, allowSeq);
    stringToSeq(reject, rejectSeq);
    StringFilterManagerIPtr adapterIdFilter = new StringFilterManagerI(allowSeq, rejectSeq, acceptOverride);

    //
    // TODO: Object id's from configurations?
    // 
    IdentitySeq allowIdSeq;
    IdentitySeq rejectIdSeq;
    allow = props->getProperty("Glacier2.Filter.AdapterId.Accept");
    reject = props->getProperty("Glacier2.Filter.AdapterId.Reject");
    acceptOverride = props->getPropertyAsIntWithDefault("Glacier2.Filter.AdapterId.AcceptOverride", 0) == 1;
    stringToSeq(allow, allowSeq);
    stringToSeq(reject, rejectSeq);
    IdentityFilterManagerIPtr identityFilter = new IdentityFilterManagerI(allowIdSeq, rejectIdSeq, false);

    return new ClientBlobject(communicator, new ClientBlobjectImpl(categoryFilter, adapterIdFilter, identityFilter));
}

StringFilterManagerPtr 
ClientBlobject::categoryFilter()
{
    return _impl->categoryFilter();
}

StringFilterManagerPtr 
ClientBlobject::adapterIdFilter()
{
    return _impl->adapterIdFilter();
}

IdentityFilterManagerPtr
ClientBlobject::identityFilter()
{
    return _impl->identityFilter();
}
