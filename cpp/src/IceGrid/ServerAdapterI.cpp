// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
                               bool enabled) :
    _node(node),
    _this(proxy),
    _serverId(serverName),
    _id(id),
    _server(server),
    _enabled(enabled)
{
}

ServerAdapterI::~ServerAdapterI()
{
    assert(_activateCB.empty());
}

void
ServerAdapterI::activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current&)
{
    {
        Lock sync(*this);
        if(_enabled && _proxy)
        {
            //
            // Return the adapter direct proxy.
            //
            cb->ice_response(_proxy);
            return;
        }
        else if(_activateCB.empty())
        {
            //
            // Nothing else waits for this adapter so we must make sure that this
            // adapter if still activatable.
            //
            if(!_enabled || !_server->isAdapterActivatable(_id))
            {
                cb->ice_response(0);
                return;
            }
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
        _activateAfterDeactivating = _server->getState(Ice::emptyCurrent) >= Deactivating &&
            _server->getState(Ice::emptyCurrent) < Destroying;
    }

    //
    // Try to start the server. Note that we start the server outside
    // the synchronization block since start() can block and callback
    // on this adapter (when the server is deactivating for example).
    //
    try
    {
        _server->start(ServerI::OnDemand);
        return;
    }
    catch(const ServerStartException& ex)
    {
        activationFailed(ex.reason);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        //
        // The server associated to this adapter doesn't exist anymore. Somehow the database is
        // inconsistent if this happens. The best thing to do is to destroy the adapter.
        //
        destroy();
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << "unexpected exception: " << ex;
        activationFailed(os.str());
    }
}

Ice::ObjectPrx
ServerAdapterI::getDirectProxy(const Ice::Current&) const
{
    Lock sync(*this);

    //
    // Return the adapter direct proxy if it's set. Otherwise, throw. The caller can eventually
    // activate the adapter if it's activatable.
    //
    if(_proxy && _enabled)
    {
        return _proxy;
    }
    else
    {
        throw AdapterNotActiveException(_enabled && _server->isAdapterActivatable(_id));
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
    if(!_node->allowEndpointsOverride())
    {
        if(prx && _proxy)
        {
            if(_server->getState(Ice::emptyCurrent) == Active)
            {
                throw AdapterActiveException();
            }
        }
    }

    bool updated = _proxy != prx;
    _proxy = prx;

    //
    // If the server is being deactivated and the activation callback
    // was added during the deactivation, we don't send the response
    // now. The server is going to be activated again and the adapter
    // activated.
    //
    if(_server->getState(Ice::emptyCurrent) < Deactivating ||
       _server->getState(Ice::emptyCurrent) >= Destroying || !_activateAfterDeactivating)
    {
        for(vector<AMD_Adapter_activatePtr>::const_iterator p = _activateCB.begin(); p != _activateCB.end(); ++p)
        {
            (*p)->ice_response(_proxy);
        }
        _activateCB.clear();
    }

    if(updated)
    {
        AdapterDynamicInfo info;
        info.id = _id;
        info.proxy = _proxy;
        _node->observerUpdateAdapter(info);
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
    activationFailed("adapter destroyed");
    try
    {
        _node->getAdapter()->remove(_this->ice_getIdentity());
    }
    catch(const Ice::LocalException&)
    {
        // Ignore.
    }
}

void
ServerAdapterI::updateEnabled()
{
    Lock sync(*this);
    _enabled = _server->isEnabled(Ice::emptyCurrent);
}

void
ServerAdapterI::clear()
{
    Lock sync(*this);
    _proxy = 0;
    _activateAfterDeactivating = false;
}

void
ServerAdapterI::activationFailed(const std::string& reason)
{

    //
    // The server couldn't be activated, trace and return the current adapter proxy.
    //
    if(_node->getTraceLevels()->adapter > 1)
    {
        Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
        out << "server `" + _serverId + "' adapter `" << _id << "' activation failed: " << reason;
    }

    Lock sync(*this);
    for(vector<AMD_Adapter_activatePtr>::const_iterator p = _activateCB.begin(); p != _activateCB.end(); ++p)
    {
        (*p)->ice_response(0);
    }
    _activateCB.clear();
}

void
ServerAdapterI::activationCompleted()
{
    Lock sync(*this);
    if(!_proxy)
    {
        //
        // The server activation completed, but the adapter hasn't been activated.
        //
        if(_node->getTraceLevels()->adapter > 1)
        {
            Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
            out << "server `" + _serverId + "' adapter `" << _id << "' activation failed: server activation completed";
        }
    }

    for(vector<AMD_Adapter_activatePtr>::const_iterator p = _activateCB.begin(); p != _activateCB.end(); ++p)
    {
        (*p)->ice_response(_proxy);
    }
    _activateCB.clear();
}

AdapterPrx
ServerAdapterI::getProxy() const
{
    return _this;
}
