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

    AMI_Adapter_getDirectProxyI(const Ice::AMD_Locator_findAdapterByIdPtr& cb, 
				const AdapterRegistryPtr& registry,
				const string& id,
				const AdapterPrx& adapter) : 
	_cb(cb), _adapterRegistry(registry), _id(id), _adapter(adapter)
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
	    // Expected if the adapter is destroyed, if that's the case, we remove it from the adapter
	    // registry.
	    //
	    try
	    {
		_adapterRegistry->remove(_id, _adapter);
	    }
	    catch(const AdapterNotExistException&)
	    {
	    }
	    _cb->ice_exception(Ice::AdapterNotFoundException());
	    return;
	}
	catch(const Ice::LocalException&)
	{
	    //
	    // Expected if we couldn't contact the adapter object (possibly because the IcePack node is 
	    // down). Return a null proxy in this case (the client will get empty endpoints and throw a
	    // NoEndpointException).
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

    const Ice::AMD_Locator_findAdapterByIdPtr _cb;
    const AdapterRegistryPtr& _adapterRegistry;
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
IcePack::LocatorI::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& cb,
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
IcePack::LocatorI::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& cb, 
					 const string& id, 
					 const Ice::Current&) const
{
    try
    {
	AdapterPrx adapter = AdapterPrx::uncheckedCast(_adapterRegistry->findById(id));
	AMI_Adapter_getDirectProxyPtr amiCB = new AMI_Adapter_getDirectProxyI(cb, _adapterRegistry, id, adapter);
	adapter->getDirectProxy_async(amiCB, true);
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
