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
// Callback from asynchronous call to adapter->getDirectProxy()
// invoked in LocatorI::findAdapterById_async().
//
class AMI_Adapter_getDirectProxyI : public AMI_Adapter_getDirectProxy
{
public:

    AMI_Adapter_getDirectProxyI(const Ice::AMD_Locator_findAdapterByIdPtr& cb) : _cb(cb)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
	//
	// Return the adapter dummy direct proxy.
	//
	_cb->ice_response(obj);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    //
	    // Expected if the adapter is destroyed.
	    //
	    _cb->ice_exception(Ice::AdapterNotFoundException());
	    return;
	}
	catch(const Ice::LocalException&)
	{
	    //
	    // Expected if we couldn't contact the adapter object
	    // (possibly because the IcePack node is down). Return a
	    // null proxy in this case (the client will get empty
	    // endpoints and throw a NoEndpointException).
	    //
	    _cb->ice_response(0);
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

private:

    Ice::AMD_Locator_findAdapterByIdPtr _cb;
};

//
// Callback from asynchrnous call to LocatorI::findAdapterById_async()
// invoked in LocatorI::findObjectById_async().
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
    
    Ice::AMD_Locator_findObjectByIdPtr _cb;
    Ice::ObjectPrx _obj;

};

}

IcePack::LocatorI::LocatorI(const AdapterRegistryPtr& adapterRegistry, 
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
IcePack::LocatorI::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& response, const Ice::Identity& id, 
					const Ice::Current& current) const
{
    ObjectDescription obj;
    try
    {
	obj = _objectRegistry->getObjectDescription(id);
    }
    catch(const ObjectNotExistException&)
    {
	throw Ice::ObjectNotFoundException();
    }

    //
    // OPTIMIZATION: If the object is registered with an adapter id,
    // try to get the adapter direct proxy (which might caused the
    // server activation). This will avoid the client to lookup for
    // the adapter id endpoints.
    //
    if(!obj.adapterId.empty())
    {
	Ice::AMD_Locator_findAdapterByIdPtr cb = new AMD_Locator_findAdapterByIdI(response, obj.proxy);
	findAdapterById_async(cb, obj.adapterId, current);
    }
    else
    {
	response->ice_response(obj.proxy);
    }
}
    
//
// Find an adapter by identity. The object is searched in the adapter
// registry. If found, we try to get its direct proxy.
//
void
IcePack::LocatorI::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& response,
					 const string& id, const Ice::Current&) const
{
    try
    {
	AMI_Adapter_getDirectProxyPtr cb = new AMI_Adapter_getDirectProxyI(response);
	_adapterRegistry->findById(id)->getDirectProxy_async(cb, true);
    }
    catch(const AdapterNotExistException&)
    {
	throw Ice::AdapterNotFoundException();
    }
}

Ice::LocatorRegistryPrx
IcePack::LocatorI::getRegistry(const Ice::Current&) const
{
    return _locatorRegistry;
}
