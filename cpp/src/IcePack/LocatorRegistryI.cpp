// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/AdapterFactory.h>

using namespace std;
using namespace IcePack;

class AMI_Adapter_setDirectProxyI : public AMI_Adapter_setDirectProxy
{
public:

    AMI_Adapter_setDirectProxyI(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb) : _cb(cb)
    {
    }

    virtual void ice_response()
    {
	_cb->ice_response();
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const AdapterActiveException&)
	{
	    _cb->ice_exception(Ice::AdapterAlreadyActiveException());
	    return;
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    _cb->ice_exception(Ice::AdapterNotFoundException()); // Expected if the adapter was destroyed.
	    return;
	}
	catch(const Ice::LocalException&)
	{
	    _cb->ice_response();
	    return;
	}

	assert(false);
    }

private:

    Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr _cb;
};

class AMI_Server_setProcessI : public AMI_Server_setProcess
{
public:

    AMI_Server_setProcessI(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb) : _cb(cb)
    {
    }

    virtual void ice_response()
    {
	_cb->ice_response();
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    _cb->ice_exception(Ice::ServerNotFoundException()); // Expected if the adapter was destroyed.
	    return;
	}
	catch(const Ice::LocalException&)
	{
	    _cb->ice_response();
	    return;
	}

	assert(false);
    }

private:

    Ice::AMD_LocatorRegistry_setServerProcessProxyPtr _cb;
};

IcePack::LocatorRegistryI::LocatorRegistryI(const AdapterRegistryPtr& adapterRegistry, 
                                            const ServerRegistryPtr& serverRegistry,
					    const AdapterFactoryPtr& adapterFactory,
					    bool dynamicRegistration) :
    _adapterRegistry(adapterRegistry),
    _serverRegistry(serverRegistry),
    _adapterFactory(adapterFactory),
    _dynamicRegistration(dynamicRegistration)
{
}

void 
IcePack::LocatorRegistryI::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
						       const string& id, 
						       const Ice::ObjectPrx& proxy,
						       const Ice::Current&)
{
    while(true)
    {
	try
	{
	    //
	    // Get the adapter from the registry and set its direct proxy.
	    //
	    AMI_Adapter_setDirectProxyPtr amiCB = new AMI_Adapter_setDirectProxyI(cb);
	    _adapterRegistry->findById(id)->setDirectProxy_async(amiCB, proxy);
	    return;
	}
	catch(const AdapterNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	    cb->ice_response();
	    return;
	}

	if(_dynamicRegistration)
	{
	    //
	    // Create a new standalone adapter. The adapter will be persistent. It's the responsability of 
	    // the user to cleanup adapter entries which were dynamically added from the registry.
	    //
	    AdapterPrx adapter = _adapterFactory->createStandaloneAdapter(id);
	    try
	    {
		_adapterRegistry->add(id, adapter);
	    }
	    catch(const AdapterExistsException&)
	    {
		adapter->destroy();
	    }
	}
	else
	{
	    throw Ice::AdapterNotFoundException();
	}
    }
}

void
IcePack::LocatorRegistryI::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
						       const string& name,
						       const Ice::ProcessPrx& proxy, 
						       const Ice::Current&)
{
    try
    {
        //
        // Get the server from the registry and set its process proxy.
        //
	AMI_Server_setProcessPtr amiCB = new AMI_Server_setProcessI(cb);
        _serverRegistry->findByName(name)->setProcess_async(amiCB, proxy);
        return;
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const Ice::LocalException&)
    {
        //
        // TODO: We couldn't contact the server object. This is possibly because the IcePack node is down and
        // the server is started manually for example. We should probably throw here to prevent the server
        // from starting?
        //
	cb->ice_response();
	return;
    }

    throw Ice::ServerNotFoundException();
}
