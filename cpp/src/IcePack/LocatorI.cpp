// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/LocatorI.h>

using namespace std;
using namespace IcePack;

namespace IcePack
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
	    _cb->ice_response(obj->ice_newIdentity(_obj->ice_getIdentity()));
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

LocatorI::LocatorI(const AdapterRegistryPtr& adapterRegistry, 
		   const ObjectRegistryPtr& objectRegistry, 
		   const Ice::LocatorRegistryPrx& locatorRegistry) :
    _adapterRegistry(adapterRegistry),
    _objectRegistry(objectRegistry),
    _locatorRegistry(locatorRegistry)
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
    ObjectDescriptor obj;
    try
    {
	obj = _objectRegistry->getObjectDescriptor(id);
    }
    catch(const ObjectNotExistException&)
    {
	throw Ice::ObjectNotFoundException();
    }

    //
    // OPTIMIZATION: If the object is registered with an adapter id, try to get the adapter direct
    // proxy (which might caused the server activation). This will avoid the client to lookup for
    // the adapter id endpoints.
    //
    if(!obj.adapterId.empty())
    {
	Ice::AMD_Locator_findAdapterByIdPtr amiCB = new AMD_Locator_findAdapterByIdI(cb, obj.proxy);
	findAdapterById_async(amiCB, obj.adapterId, current);
    }
    else
    {
	cb->ice_response(obj.proxy);
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
    AdapterPrx adapter;
    try
    {
	adapter = AdapterPrx::uncheckedCast(_adapterRegistry->findById(id));
    }
    catch(const AdapterNotExistException&)
    {
	throw Ice::AdapterNotFoundException();
    }

    LocatorIPtr self = const_cast<LocatorI*>(this);
    if(self->getDirectProxyRequest(cb, adapter))
    {
	try
	{
	    AMI_Adapter_getDirectProxyPtr amiCB = new AMI_Adapter_getDirectProxyI(self, id, adapter);
	    adapter->getDirectProxy_async(amiCB);
	}
	catch(const Ice::LocalException& ex)
	{
	    self->getDirectProxyException(adapter, id, ex);
	}
    }
}

Ice::LocatorRegistryPrx
LocatorI::getRegistry(const Ice::Current&) const
{
    return _locatorRegistry;
}

bool
LocatorI::getDirectProxyRequest(const Ice::AMD_Locator_findAdapterByIdPtr& cb, const AdapterPrx& adapter)
{
    Lock sync(*this);
    
    //
    // Check if there's already pending requests for this adapter. If that's the case,
    // we just add this one to the queue. If not, we add it to the queue and initiate
    // a call on the adapter to get its direct proxy.
    //
    PendingRequestsMap::iterator p;
    p = _pendingRequests.insert(make_pair(adapter->ice_getIdentity(), PendingRequests())).first;
    p->second.push_back(cb);
    return p->second.size() == 1;
}

void
LocatorI::getDirectProxyException(const AdapterPrx& adapter, const string& id, const Ice::Exception& ex)
{
    Lock sync(*this);

    PendingRequestsMap::iterator p = _pendingRequests.find(adapter->ice_getIdentity());
    assert(p != _pendingRequests.end());
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
	    try
	    {
		AMI_Adapter_activatePtr amiCB = new AMI_Adapter_activateI(this, id, adapter);
		AdapterPrx::uncheckedCast(adapter->ice_timeout(ex.timeout))->activate_async(amiCB);
	    }
	    catch(const Ice::LocalException& ex)
	    {
		getDirectProxyException(adapter, id, ex);
	    }
	    return;
	}
	else
	{
	    for(PendingRequests::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	    {
		(*q)->ice_response(0);
	    }
	}
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// Expected if the adapter is destroyed, if that's the case, we remove it from the adapter registry.
	//
	try
	{
	    _adapterRegistry->remove(id, adapter);
	}
	catch(const AdapterNotExistException&)
	{
	}

	for(PendingRequests::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    (*q)->ice_exception(Ice::AdapterNotFoundException());
	}
    }
    catch(const Ice::LocalException&)
    {
	for(PendingRequests::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    (*q)->ice_response(0);
	}
    }
    catch(const Ice::Exception&)
    {
	assert(false);
    }
    _pendingRequests.erase(p);
}

void
LocatorI::getDirectProxyCallback(const Ice::Identity& adapterId, const Ice::ObjectPrx& proxy)
{
    Lock sync(*this);

    PendingRequestsMap::iterator p = _pendingRequests.find(adapterId);
    assert(p != _pendingRequests.end());
    for(PendingRequests::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
    {
	(*q)->ice_response(proxy);
    }
    _pendingRequests.erase(p);
}
