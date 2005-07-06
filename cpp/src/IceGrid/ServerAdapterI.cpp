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
#include <IceGrid/ServerI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/NodeI.h>

using namespace std;
using namespace IceGrid;

ServerAdapterI::ServerAdapterI(const NodeIPtr& node,
			       const ServerIPtr& server, 
			       const AdapterPrx& proxy,
			       const string& id,
			       Ice::Int waitTime) :
    _node(node),
    _this(proxy),
    _id(id),
    _server(server),
    _waitTime(IceUtil::Time::seconds(waitTime))
{
}

ServerAdapterI::~ServerAdapterI()
{
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

	_activateCB.push_back(cb);
	if(_activateCB.size() > 1)
	{
	    return;
	}
    }

    //
    // Try to start the server. Note that we start the server outside
    // the synchronization block since start() can block and callback
    // on this adapter (when the server is deactivating for example).
    //
    try
    {
	if(_server->startInternal(OnDemand))
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
    
    activationFailed(false);
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

    bool updated = _proxy != prx;
    _proxy = prx;

    for(vector<AMD_Adapter_activatePtr>::const_iterator p = _activateCB.begin(); p != _activateCB.end(); ++p)
    {
	(*p)->ice_response(_proxy);
    }
    _activateCB.clear();

    if(updated)
    {
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
    }

    if(_proxy)
    {
	_server->adapterActivated(_id);
    }
    else
    {
	_server->adapterDeactivated(_id);
    }

    if(_node->getTraceLevels()->adapter > 1)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
	out << "server adapter `" << _id << "' " << (_proxy ? "activated" : "deactivated");
    }
}

void
ServerAdapterI::destroy(const Ice::Current& current)
{
    assert(current.adapter);
    current.adapter->remove(_this->ice_getIdentity());
}

void 
ServerAdapterI::activationFailed(bool timeout)
{

    //
    // The server couldn't be activated, trace and return the current adapter proxy.
    //
    if(_node->getTraceLevels()->adapter > 1)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
	if(timeout)
	{
	    out << "server adapter `" << _id << "' activation timed out";
	}
	else
	{
	    out << "server adapter `" << _id << "' activation failed, couldn't start the server";
	}
    }

    for(vector<AMD_Adapter_activatePtr>::const_iterator p = _activateCB.begin(); p != _activateCB.end(); ++p)
    {
	(*p)->ice_response(0);
    }
    _activateCB.clear();
}
