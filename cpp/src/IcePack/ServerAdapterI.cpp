// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ServerAdapterI.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/TraceLevels.h>
#include <IcePack/WaitQueue.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class WaitForAdapterActivation : public WaitItem
{
public:
 
    WaitForAdapterActivation(const ServerAdapterPtr& adapter, 
			     const TraceLevelsPtr traceLevels,
			     const AMD_Adapter_activatePtr& cb) : 
	WaitItem(adapter),
	_adapter(adapter),
	_traceLevels(traceLevels),
	_cb(cb)
    {
    }
    
    virtual void execute()
    {
	try
	{
	    _cb->ice_response(_adapter->getDirectProxy());
	}
	catch(const AdapterNotActiveException&)
	{
	    _cb->ice_response(0);
	}
	catch(const Ice::LocalException&)
	{
	    _cb->ice_response(0);
	}
    }

    virtual void expired(bool destroyed)
    {
	if(_traceLevels->adapter > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	    out << "server adapter `" << _adapter->id << "' activation timed out";
	}
	_cb->ice_response(0);
    }

private:
    
    const ServerAdapterPtr _adapter;
    const TraceLevelsPtr _traceLevels;
    const AMD_Adapter_activatePtr _cb;
};

}

ServerAdapterI::ServerAdapterI(const ServerFactoryPtr& factory, const TraceLevelsPtr& traceLevels, 
					Ice::Int waitTime) :
    _factory(factory),
    _traceLevels(traceLevels),
    _waitTime(IceUtil::Time::seconds(waitTime))
{
}

ServerAdapterI::~ServerAdapterI()
{
}

string
ServerAdapterI::getId(const Ice::Current&)
{
    return id;
}

void
ServerAdapterI::activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current& current)
{
    {
	Lock sync(*this);
	if(_proxy)
	{
	    //
	    // Return the adapter direct proxy.
	    //
	    cb->ice_response(_proxy);
	    return;
	}

	if(_traceLevels->adapter > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	    out << "waiting for activation of server adapter `" << id << "'";
	}

	_factory->getWaitQueue()->add(new WaitForAdapterActivation(this, _traceLevels, cb), _waitTime);
    }

    //
    // Try to start the server. Note that we start the server outside
    // the synchronization block since start() can block and callback
    // on this adapter (when the server is deactivating for example).
    //
    try
    {
	if(svr->start(OnDemand))
	{
	    return;
	}
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// The server associated to this adapter doesn't exist anymore. Somehow the database is 
	// inconsistent if this happens. The best thing to do is to destroy the adapter and throw
	// an ObjectNotExist exception.
	//
	destroy(current);
    }

    //
    // The server couldn't be activated, trace and return the current adapter proxy.
    //
    if(_traceLevels->adapter > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "server adapter `" << id << "' activation failed, couldn't start the server";
    }
    
    _factory->getWaitQueue()->notifyAllWaitingOn(this);
}

Ice::ObjectPrx
ServerAdapterI::getDirectProxy(const Ice::Current& current) const
{
    Lock sync(*this);

    //
    // Return the adapter direct proxy if it's set. Otherwise, throw. The caller can eventually
    // activate the adapter if it's activatable.
    //
    if(_proxy)
    {
	return _proxy;
    }
    else
    {
	AdapterNotActiveException ex;
	ServerState state = svr->getState();
	ex.activatable = svr->getActivationMode() == OnDemand || state == Activating || state == Active;
	ex.timeout = static_cast<int>(_waitTime.toMilliSeconds());
	throw ex;
    }
}

void
ServerAdapterI::setDirectProxy(const Ice::ObjectPrx& prx, const Ice::Current& current)
{
    Lock sync(*this);

    //
    // If the adapter proxy is not null the given proxy can only be null. We don't allow to overide an 
    // existing proxy by another non null proxy if the server is active.
    //
    if(prx && _proxy)
    {
	if(svr->getState() == Active)
	{
	    throw AdapterActiveException();
	}
    }

    //
    // Prevent eviction of an active adapter object.
    //
    if(prx && !_proxy)
    {
	_factory->getServerAdapterEvictor()->keep(current.id);
    }
    else if(!prx && _proxy)
    {
	_factory->getServerAdapterEvictor()->release(current.id);
    }

    _proxy = prx;

    if(_traceLevels->adapter > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "server adapter `" << id << "' " << (_proxy ? "activated" : "deactivated");
    }
    
    _factory->getWaitQueue()->notifyAllWaitingOn(this);
}

void
ServerAdapterI::destroy(const Ice::Current& current)
{
    _factory->destroy(this, current.id);
}
