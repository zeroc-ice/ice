// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

//
// Callback from asynchronous call to adapter->getDirectProxy() invoked in LocatorI::findAdapterById_async().
//
class AMI_Adapter_getDirectProxyI : public AMI_Adapter_getDirectProxy
{
public:

    AMI_Adapter_getDirectProxyI(const LocatorIPtr& locator, const string& id, const AdapterPrx& adapter) : 
	_locator(locator), _id(id), _adapter(adapter)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
	assert(obj);
	_locator->getDirectProxyCallback(_adapter->ice_getIdentity(), obj);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
	_locator->getDirectProxyException(_adapter, _id, ex);
    }

private:

    const LocatorIPtr _locator;
    const string _id;
    const AdapterPrx _adapter;
};

class AMI_Adapter_activateI : public AMI_Adapter_activate
{
public:

    AMI_Adapter_activateI(const LocatorIPtr& locator, const string& id, const AdapterPrx& adapter) : 
	_locator(locator), _id(id), _adapter(adapter)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
	_locator->getDirectProxyCallback(_adapter->ice_getIdentity(), obj);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
	_locator->getDirectProxyException(_adapter, _id, ex);
    }

private:

    const LocatorIPtr _locator;
    const string _id;
    const AdapterPrx _adapter;
};

//
// Callback from asynchrnous call to LocatorI::findAdapterById_async() invoked in LocatorI::findObjectById_async().
//
class AMD_Locator_findAdapterByIdI : public Ice::AMD_Locator_findAdapterById
{
public:

    AMD_Locator_findAdapterByIdI(const Ice::AMD_Locator_findObjectByIdPtr& cb, const Ice::ObjectPrx& obj) : 
	_cb(cb),
	_obj(obj)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
	//
	// If the adapter dummy direct proxy is not null, return a
	// proxy containing the identity we were looking for and the
	// endpoints of the adapter.
	//
	// If null, return the proxy registered with the object
	// registry.
	//
	if(obj)
	{
	    _cb->ice_response(obj->ice_identity(_obj->ice_getIdentity()));
	}
	else
	{
	    _cb->ice_response(_obj);
	}
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(Ice::AdapterNotFoundException&)
	{
	    //
	    // We couldn't find the adapter, we ignore and return the
	    // original proxy containing the adapter id.
	    // 
	    _cb->ice_response(_obj);
	    return;
	}
	catch(const Ice::Exception& ex)
	{
	    //
	    // Rethrow unexpected exception.
	    //
	    _cb->ice_exception(ex);
	    return;
	}
	
	assert(false);
    }

    virtual void ice_exception(const std::exception& ex)
    {
	_cb->ice_exception(ex);
    }

    virtual void ice_exception()
    {
	_cb->ice_exception();
    }

private:
    
    const Ice::AMD_Locator_findObjectByIdPtr _cb;
    const Ice::ObjectPrx _obj;
};

}

LocatorI::Request::Request(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB, 
			   const LocatorIPtr& locator,
			   const string& id,
			   const vector<pair<string, AdapterPrx> >& adapters,
			   int count) : 
    _amdCB(amdCB),
    _locator(locator),
    _id(id),
    _adapters(adapters),
    _count(count),
    _lastAdapter(_adapters.begin())
{
    assert(_count > 0);
}

void
LocatorI::Request::execute()
{
    //
    // Request as many adapters as required.
    //
    vector<AdapterPrx> adapters;
    {
	Lock sync(*this);
	for(unsigned int i = 0; i < _count; ++i)
	{
	    if(_lastAdapter == _adapters.end())
	    {
		_count = i;
		break;
	    }
	    assert(_lastAdapter->second);
	    adapters.push_back(_lastAdapter->second);
	    ++_lastAdapter;
	}
    }
    for(vector<AdapterPrx>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	requestAdapter(*p);
    }
}

void
LocatorI::Request::exception()
{
    AdapterPrx adapter;
    {
	Lock sync(*this);
	if(_lastAdapter == _adapters.end())
	{
	    --_count; // Expect one less adapter proxy if there's no more adapters to query.

	    //
	    // If we received all the required proxies, it's time to send the
	    // answer back to the client.
	    //
	    if(_count == _proxies.size())
	    {
		sendResponse();
	    }
	    return;
	}
	else
	{
	    adapter = _lastAdapter->second;
	    ++_lastAdapter;
	}
    }
    requestAdapter(adapter);
}

void
LocatorI::Request::response(const Ice::ObjectPrx& proxy)
{
    if(!proxy)
    {
	exception();
	return;
    }

    Lock sync(*this);
    _proxies.push_back(proxy->ice_identity(Ice::stringToIdentity("dummy")));

    //
    // If we received all the required proxies, it's time to send the
    // answer back to the client.
    //
    if(_proxies.size() == _count)
    {
	sendResponse();
    }
}

void
LocatorI::Request::requestAdapter(const AdapterPrx& adapter)
{
    assert(adapter);
    if(_locator->getDirectProxyRequest(this, adapter))
    {
	AMI_Adapter_getDirectProxyPtr amiCB = new AMI_Adapter_getDirectProxyI(_locator, _id, adapter);
	adapter->getDirectProxy_async(amiCB);
    }
}

void
LocatorI::Request::sendResponse()
{
    if(_proxies.size() == 1)
    {
	_amdCB->ice_response(_proxies.back());
    }
    else if(_proxies.empty())
    {
	_amdCB->ice_response(0);
    }
    else if(_proxies.size() > 1)
    {
	Ice::EndpointSeq endpoints;
	endpoints.reserve(_proxies.size());
	for(vector<Ice::ObjectPrx>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
	{
	    Ice::EndpointSeq edpts = (*p)->ice_getEndpoints();
	    endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
	}

	Ice::ObjectPrx proxy = _locator->getCommunicator()->stringToProxy("dummy:default");
	_amdCB->ice_response(proxy->ice_endpoints(endpoints));
    }
}

LocatorI::LocatorI(const Ice::CommunicatorPtr& communicator, 
		   const DatabasePtr& database, 
		   const Ice::LocatorRegistryPrx& locatorRegistry) :
    _communicator(communicator),
    _database(database),
    _locatorRegistry(Ice::LocatorRegistryPrx::uncheckedCast(locatorRegistry->ice_collocationOptimization(false)))
{
}

//
// Find an object by identity. The object is searched in the object
// registry.
//
void
LocatorI::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& cb, 
			       const Ice::Identity& id, 
			       const Ice::Current& current) const
{
    Ice::ObjectPrx proxy;
    try
    {
	proxy = _database->getObjectProxy(id);
    }
    catch(const ObjectNotRegisteredException&)
    {
	throw Ice::ObjectNotFoundException();
    }

    //
    // OPTIMIZATION: If the object is registered with an adapter id,
    // try to get the adapter direct proxy (which might caused the
    // server activation). This will avoid the client to lookup for
    // the adapter id endpoints.
    //
    const string adapterId = proxy->ice_getAdapterId();
    if(!adapterId.empty())
    {
	Ice::AMD_Locator_findAdapterByIdPtr amiCB = new AMD_Locator_findAdapterByIdI(cb, proxy);
	findAdapterById_async(amiCB, adapterId, current);
    }
    else
    {
	cb->ice_response(proxy);
    }
}
    
//
// Find an adapter by identity. The object is searched in the adapter
// registry. If found, we try to get its direct proxy.
//
void
LocatorI::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& cb, 
				const string& id, 
				const Ice::Current&) const
{
    try
    {
	int count;
	vector<pair<string, AdapterPrx> > adapters = _database->getAdapters(id, false, count);
	if(adapters.empty())
	{
	    cb->ice_response(0);
	    return;
	}
	(new Request(cb, const_cast<LocatorI*>(this), id, adapters, count))->execute();
    }
    catch(const AdapterNotExistException&)
    {
	cb->ice_exception(Ice::AdapterNotFoundException());
	return;
    }
}

Ice::LocatorRegistryPrx
LocatorI::getRegistry(const Ice::Current&) const
{
    return _locatorRegistry;
}

bool
LocatorI::getDirectProxyRequest(const RequestPtr& request, const AdapterPrx& adapter)
{
    Lock sync(*this);

    //
    // Check if there's already pending requests for this adapter. If that's the case,
    // we just add this one to the queue. If not, we add it to the queue and initiate
    // a call on the adapter to get its direct proxy.
    //
    PendingRequests& requests = _pendingRequests[adapter->ice_getIdentity()];
    requests.push_back(request);
    return requests.size() == 1;
}

void
LocatorI::getDirectProxyException(const AdapterPrx& adapter, const string& id, const Ice::Exception& ex)
{
    try
    {
	ex.ice_throw();
    }
    catch(const AdapterNotActiveException& ex)
    {
	if(ex.activatable)
	{
	    //
	    // Activate the adapter if it can be activated on demand. NOTE: we use the timeout
	    // provided in the exception to activate the adapter. The timeout correspond to the
	    // wait time configured for the server.
	    //
	    AMI_Adapter_activatePtr amiCB = new AMI_Adapter_activateI(this, id, adapter);
	    AdapterPrx::uncheckedCast(adapter->ice_timeout(ex.timeout))->activate_async(amiCB);
	    return;
	}
    }
    catch(const Ice::Exception& ex)
    {
	//
	// TODO: Add a warning!!!
	//
    }

    PendingRequests requests;
    {
	Lock sync(*this);
	PendingRequestsMap::iterator p = _pendingRequests.find(adapter->ice_getIdentity());
	assert(p != _pendingRequests.end());
	requests = p->second;
	_pendingRequests.erase(p);
    }

    for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
    {
	(*q)->exception();
    }
}

void
LocatorI::getDirectProxyCallback(const Ice::Identity& adapterId, const Ice::ObjectPrx& proxy)
{
    PendingRequests requests;
    {
	Lock sync(*this);
	PendingRequestsMap::iterator p = _pendingRequests.find(adapterId);
	assert(p != _pendingRequests.end());
	requests = p->second;
	_pendingRequests.erase(p);
    }

    for(PendingRequests::const_iterator q = requests.begin(); q != requests.end(); ++q)
    {
	(*q)->response(proxy);
    }
}

const Ice::CommunicatorPtr&
LocatorI::getCommunicator() const
{
    return _communicator;
}
