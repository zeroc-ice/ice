// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

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

LocatorRegistryI::LocatorRegistryI(const DatabasePtr& database, bool dynamicRegistration) :
    _database(database),
    _dynamicRegistration(dynamicRegistration)
{
}

void 
LocatorRegistryI::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
					      const string& serverId,
					      const string& adapterId, 
					      const Ice::ObjectPrx& proxy,
					      const Ice::Current&)
{
    try
    {
	//
	// Get the adapter from the registry and set its direct proxy.
	//
	AMI_Adapter_setDirectProxyPtr amiCB = new AMI_Adapter_setDirectProxyI(cb);
	_database->getAdapter(adapterId, serverId)->setDirectProxy_async(amiCB, proxy);
	return;
    }
    catch(const ServerNotExistException&)
    {
	if(!_dynamicRegistration)
	{
	    throw Ice::ServerNotFoundException();
	}
    }
    catch(const AdapterNotExistException&)
    {
	if(!_dynamicRegistration)
	{
	    throw Ice::AdapterNotFoundException();
	}
    }
    catch(const Ice::LocalException&)
    {
	cb->ice_response();
	return;
    }
    
    assert(_dynamicRegistration);
    try
    {
	_database->setAdapterDirectProxy(serverId, adapterId, proxy);
    }
    catch(const Ice::Exception&)
    {
    }
    cb->ice_response();
}

void
LocatorRegistryI::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
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
        _database->getServer(name)->setProcess_async(amiCB, proxy);
        return;
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const Ice::LocalException&)
    {
        //
        // TODO: We couldn't contact the server object. This is possibly because the IceGrid node is down and
        // the server is started manually for example. We should probably throw here to prevent the server
        // from starting?
        //
	cb->ice_response();
	return;
    }

    throw Ice::ServerNotFoundException();
}
