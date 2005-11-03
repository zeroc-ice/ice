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
			       ServerI* server, 
			       const string& serverName,
			       const AdapterPrx& proxy,
			       const string& id,
			       Ice::Int waitTime) :
    _node(node),
    _this(proxy),
    _serverId(serverName),
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
	    out << "waiting for activation of server `" + _serverId + "' adapter `" << _id << "'";
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
	_server->start_async(0);
	return;
    }
    catch(const ServerStartException&)
    {
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// The server associated to this adapter doesn't exist anymore. Somehow the database is 
	// inconsistent if this happens. The best thing to do is to destroy the adapter and throw
	// an ObjectNotExist exception.
	//
	destroy();
    }
    
    activationFailed(_server->getState() != IceGrid::Activating);
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
	ex.activatable = _server->getActivationMode() == ServerI::OnDemand || state == Activating || state == Active;
	ex.timeout = static_cast<int>(_waitTime.toMilliSeconds());
	throw ex;
    }
}

void
ServerAdapterI::setDirectProxy(const Ice::ObjectPrx& prx, const Ice::Current&)
{
    Lock sync(*this);

    //
    // We don't allow to override an existing proxy by another non
    // null proxy if the server is not inactive.
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
		observer->updateAdapter(_node->getName(), info);
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
	out << "server `" + _serverId + "' adapter `" << _id << "' " << (_proxy ? "activated" : "deactivated");
	if(_proxy)
	{
	    out << ": " << _node->getCommunicator()->proxyToString(_proxy);
	}
    }
}

void
ServerAdapterI::destroy()
{
    _node->getAdapter()->remove(_this->ice_getIdentity());
}

void
ServerAdapterI::clear()
{
    Lock sync(*this);
    _proxy = 0;
}

void 
ServerAdapterI::activationFailed(bool destroyed)
{

    //
    // The server couldn't be activated, trace and return the current adapter proxy.
    //
    if(_node->getTraceLevels()->adapter > 1)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
	if(!destroyed)
	{
	    out << "server `" + _serverId + "' adapter `" << _id << "' activation timed out";
	}
	else
	{
	    out << "server `" + _serverId + "' adapter `" << _id << "' activation failed: server didn't start";
	}
    }

    for(vector<AMD_Adapter_activatePtr>::const_iterator p = _activateCB.begin(); p != _activateCB.end(); ++p)
    {
	(*p)->ice_response(0);
    }
    _activateCB.clear();
}

AdapterPrx
ServerAdapterI::getProxy() const
{
    return _this;
}
