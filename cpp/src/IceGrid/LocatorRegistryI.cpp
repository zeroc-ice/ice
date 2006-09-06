// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class SetDirectProxyCB : public AMI_Adapter_setDirectProxy
{
public:

    SetDirectProxyCB(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb) : _cb(cb)
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

class SetDirectProxyForReplicatedAdapterCB : public AMI_Adapter_setDirectProxy
{
public:

    SetDirectProxyForReplicatedAdapterCB(const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb) : 
	_cb(cb)
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

    Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr _cb;
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

template<class AmdCB>
class MasterSetDirectProxyCB : public AMI_ReplicaSession_setAdapterDirectProxy
{
public:

    MasterSetDirectProxyCB(const AmdCB& cb) : 
	_cb(cb)
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
	catch(const AdapterNotExistException&)
	{
	    _cb->ice_exception(Ice::AdapterNotFoundException());
	    return;
	}
	catch(const Ice::UserException& ex)
	{
	    _cb->ice_exception(ex);
	    return;
	}
	catch(const Ice::LocalException&) // Master unreachable.
	{
	    //
	    // TODO: Add a better exception?
	    // 
	    _cb->ice_exception(Ice::AdapterNotFoundException());
	    return;
	}
	assert(false);
    }

private:

    AmdCB _cb;
};

template<class AmdCB> MasterSetDirectProxyCB<AmdCB>*
newMasterSetDirectProxyCB(const AmdCB& cb)
{
    return new MasterSetDirectProxyCB<AmdCB>(cb);
}

};

LocatorRegistryI::LocatorRegistryI(const DatabasePtr& database,
				   bool dynamicRegistration, 
				   bool master,
				   ReplicaSessionManager& session) :
    _database(database),
    _dynamicRegistration(dynamicRegistration),
    _master(master),
    _session(session)
{
}

void 
LocatorRegistryI::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
					      const string& adapterId, 
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
	    // NOTE: We pass false to the getAdapter call to indicate
	    // that we don't necessarily want an up-to-date adapter
	    // proxy. This is needed for the session activation mode
	    // for cases where the server is released during the
	    // server startup.
	    //
	    AMI_Adapter_setDirectProxyPtr amiCB = new SetDirectProxyCB(cb);
	    _database->getAdapter(adapterId, "", false)->setDirectProxy_async(amiCB, proxy);
	    const TraceLevelsPtr traceLevels = _database->getTraceLevels();
	    if(traceLevels->locator > 1)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
		out << "registered adapter `" << adapterId << "' endpoints: `" 
		    << (proxy ? proxy->ice_toString() : string("")) << "'";
	    }
	    return;
	}
	catch(const AdapterNotExistException&)
	{
	    if(!_dynamicRegistration)
	    {
		throw Ice::AdapterNotFoundException();
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    const TraceLevelsPtr traceLevels = _database->getTraceLevels();
	    if(traceLevels->locator > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
		out << "couldn't register adapter `" << adapterId << "' endpoints:\n" << toString(ex);
	    }
	    cb->ice_response();
	    return;
	}

	assert(_dynamicRegistration);
	if(_master)
	{
	    if(_database->setAdapterDirectProxy(adapterId, "", proxy))
	    {
		cb->ice_response();
		return;
	    }
	}
	else
	{
	    ReplicaSessionPrx session = _session.getSession();
	    if(session)
	    {
		session->setAdapterDirectProxy_async(newMasterSetDirectProxyCB(cb), adapterId, "", proxy);
	    }
	    else
	    {
		//
		// TODO: Add a better exception?
		//
		cb->ice_exception(Ice::AdapterNotFoundException());
	    }
	    return;
	}
    }
}

void 
LocatorRegistryI::setReplicatedAdapterDirectProxy_async(
    const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const string& adapterId, 
    const string& replicaGroupId,
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
	    AMI_Adapter_setDirectProxyPtr amiCB = new SetDirectProxyForReplicatedAdapterCB(cb);
	    _database->getAdapter(adapterId, replicaGroupId, false)->setDirectProxy_async(amiCB, proxy);
	    const TraceLevelsPtr traceLevels = _database->getTraceLevels();
	    if(traceLevels->locator > 1)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
		out << "registered replicated adapter `" << adapterId << "' endpoints: `" 
		    << (proxy ? proxy->ice_toString() : string("")) << "'";
	    }
	    return;
	}
	catch(const AdapterNotExistException&)
	{
	    if(!_dynamicRegistration)
	    {
		throw Ice::AdapterNotFoundException();
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    const TraceLevelsPtr traceLevels = _database->getTraceLevels();
	    if(traceLevels->locator > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
		out << "couldn't register replicated adapter `" << adapterId << "' endpoints:\n" << toString(ex);
	    }	    
	    cb->ice_response();
	    return;
	}
	
	assert(_dynamicRegistration);
	if(_master)
	{
	    if(_database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy))
	    {
		cb->ice_response();
		return;
	    }
	}
	else
	{
	    ReplicaSessionPrx session = _session.getSession();
	    if(session)
	    {
		session->setAdapterDirectProxy_async(newMasterSetDirectProxyCB(cb), adapterId, replicaGroupId, proxy);
	    }
	    else
	    {
		//
		// TODO: Add a better exception?
		//
		cb->ice_exception(Ice::AdapterNotFoundException());
	    }
	    return;
	}
    }
}

void
LocatorRegistryI::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
					      const string& id,
					      const Ice::ProcessPrx& proxy, 
					      const Ice::Current&)
{
    try
    {
        //
        // Get the server from the registry and set its process proxy.
        //
	// NOTE: We pass false to the getServer call to indicate that
	// we don't necessarily want an up-to-date adapter proxy. This
	// is needed for the session activation mode for cases where
	// the server is released during the server startup.
	//
	AMI_Server_setProcessPtr amiCB = new AMI_Server_setProcessI(cb);
        _database->getServer(id, false)->setProcess_async(amiCB, proxy);

	const TraceLevelsPtr traceLevels = _database->getTraceLevels();
	if(traceLevels->locator > 1)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
	    out << "registered server `" << id << "' process proxy: `" << (proxy ? proxy->ice_toString() : string(""))
	        << "'";
	}
        return;
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const Ice::Exception& ex)
    {
	const TraceLevelsPtr traceLevels = _database->getTraceLevels();
	if(traceLevels->locator > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
	    out << "couldn't register server `" << id << "' process proxy:\n" << toString(ex);
	}
	cb->ice_response();
	return;
    }

    throw Ice::ServerNotFoundException();
}
