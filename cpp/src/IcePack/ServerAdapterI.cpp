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
			     const AMD_Adapter_getDirectProxyPtr& cb) : 
	WaitItem(adapter),
	_adapter(adapter),
	_traceLevels(traceLevels),
	_cb(cb)
    {
    }
    
    virtual void execute()
    {
	_adapter->getDirectProxy_async(_cb, false);
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
    
    ServerAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    AMD_Adapter_getDirectProxyPtr _cb;
};

}

IcePack::ServerAdapterI::ServerAdapterI(const ServerFactoryPtr& factory, const TraceLevelsPtr& traceLevels, 
					Ice::Int waitTime) :
    _factory(factory),
    _traceLevels(traceLevels),
    _waitTime(IceUtil::Time::seconds(waitTime))
{
}

IcePack::ServerAdapterI::~ServerAdapterI()
{
}

string
IcePack::ServerAdapterI::getId(const Ice::Current&)
{
    return id;
}

void
IcePack::ServerAdapterI::getDirectProxy_async(const AMD_Adapter_getDirectProxyPtr& cb,
					      bool activate, 
					      const Ice::Current& current)
{
    {
	::IceUtil::Mutex::Lock sync(*this);
	if(_proxy || !activate)
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
	    //
	    // Now that the server is activated, wait for the adapter
	    // direct proxy to be set.
	    //
	    ::IceUtil::Mutex::Lock sync(*this);
	    if(!_proxy)
	    {
		_factory->getWaitQueue()->add(new WaitForAdapterActivation(this, _traceLevels, cb), _waitTime);
		return;
	    }
	}
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// The server associated to this adapter doesn't exist
	// anymore. Somehow the database is inconsistent if this
	// happens. The best thing to do is to destroy the adapter
	// and throw an ObjectNotExist exception.
	//
	destroy(current);

	Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	ex.id = current.id;
	throw ex;
    }

    //
    // The server couldn't be activated, trace and return the current
    // adapter proxy.
    //
    {
	::IceUtil::Mutex::Lock sync(*this);
	if(_traceLevels->adapter > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	    out << "server adapter `" << id << "' activation failed, couldn't start the server";
	}

	cb->ice_response(_proxy);
    }   
}

void
IcePack::ServerAdapterI::setDirectProxy(const Ice::ObjectPrx& prx, const Ice::Current& current)
{
    ::IceUtil::Mutex::Lock sync(*this);

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
IcePack::ServerAdapterI::destroy(const Ice::Current& current)
{
    _factory->destroy(this, current.id);
}
