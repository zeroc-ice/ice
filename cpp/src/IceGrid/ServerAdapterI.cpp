// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/NodeI.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class WaitForAdapterActivation : public WaitItem
{
public:
 
    WaitForAdapterActivation(const AdapterPtr& adapter, 
			     const string& id,
			     const TraceLevelsPtr traceLevels,
			     const AMD_Adapter_activatePtr& cb) : 
	WaitItem(adapter),
	_adapter(adapter),
	_id(id),
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
	    out << "server adapter `" << _id << "' activation timed out";
	}
	_cb->ice_response(0);
    }

private:
    
    const AdapterPtr _adapter;
    const string& _id;
    const TraceLevelsPtr _traceLevels;
    const AMD_Adapter_activatePtr _cb;
};

}

ServerAdapterI::ServerAdapterI(const NodeIPtr& node,
			       const ServerPrx& server, 
			       const string& id,
			       Ice::Int waitTime) :
    _node(node),
    _id(id),
    _server(server),
    _waitTime(IceUtil::Time::seconds(waitTime))
{
}

ServerAdapterI::~ServerAdapterI()
{
}

string
ServerAdapterI::getId(const Ice::Current&)
{
    return _id;
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

	if(_node->getTraceLevels()->adapter > 2)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
	    out << "waiting for activation of server adapter `" << _id << "'";
	}

	_node->getWaitQueue()->add(new WaitForAdapterActivation(this, _id, _node->getTraceLevels(), cb), _waitTime);
    }

    //
    // Try to start the server. Note that we start the server outside
    // the synchronization block since start() can block and callback
    // on this adapter (when the server is deactivating for example).
    //
    try
    {
	if(_server->start(OnDemand))
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
    if(_node->getTraceLevels()->adapter > 1)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
	out << "server adapter `" << _id << "' activation failed, couldn't start the server";
    }
    
    _node->getWaitQueue()->notifyAllWaitingOn(this);
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
	ServerState state = _server->getState();
	ex.activatable = _server->getActivationMode() == OnDemand || state == Activating || state == Active;
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
	if(_server->getState() == Active)
	{
	    throw AdapterActiveException();
	}
    }

    _proxy = prx;

    NodeObserverPrx observer = _node->getObserver();
    if(observer)
    {
	AdapterDynamicInfo info;
	info.id = _id;
	info.proxy = _proxy;
	try
	{
	    observer->updateAdapter(_node->getName(current), info);
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    if(_node->getTraceLevels()->adapter > 1)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
	out << "server adapter `" << _id << "' " << (_proxy ? "activated" : "deactivated");
    }
    
    _node->getWaitQueue()->notifyAllWaitingOn(this);
}

void
ServerAdapterI::destroy(const Ice::Current& current)
{
    current.adapter->remove(current.id);
}
