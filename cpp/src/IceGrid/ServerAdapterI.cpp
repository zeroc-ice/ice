// Copyright (c) ZeroC, Inc.

#include "ServerAdapterI.h"
#include "Ice/Ice.h"
#include "NodeI.h"
#include "ServerI.h"
#include "TraceLevels.h"

using namespace std;
using namespace IceGrid;

ServerAdapterI::ServerAdapterI(
    const shared_ptr<NodeI>& node,
    ServerI* server,
    string serverName,
    AdapterPrx proxy,
    string id,
    bool enabled)
    : _node(node),
      _this(std::move(proxy)),
      _serverId(std::move(serverName)),
      _id(std::move(id)),
      _server(server),
      _enabled(enabled)
{
}

ServerAdapterI::~ServerAdapterI() { assert(_activateCB.empty()); }

void
ServerAdapterI::activateAsync(
    function<void(const optional<Ice::ObjectPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    {
        lock_guard lock(_mutex);
        if (_enabled && _proxy)
        {
            //
            // Return the adapter direct proxy.
            //
            response(_proxy);
            return;
        }
        else if (_activateCB.empty())
        {
            //
            // Nothing else waits for this adapter so we must make sure that this
            // adapter if still activatable.
            //
            if (!_enabled || !_server->isAdapterActivatable(_id))
            {
                response(nullopt);
                return;
            }
        }

        if (_node->getTraceLevels()->adapter > 2)
        {
            Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
            out << "waiting for activation of server '" + _serverId + "' adapter '" << _id << "'";
        }

        _activateCB.push_back(response);
        if (_activateCB.size() > 1)
        {
            return;
        }
        _activateAfterDeactivating =
            _server->getState() >= ServerState::Deactivating && _server->getState() < ServerState::Destroying;
    }

    //
    // Try to start the server. Note that we start the server outside
    // the synchronization block since start() can block and callback
    // on this adapter (when the server is deactivating for example).
    //
    try
    {
        _server->start(ServerI::ServerActivation::OnDemand);
        return;
    }
    catch (const ServerStartException& ex)
    {
        activationFailed(ex.reason);
    }
    catch (const Ice::ObjectNotExistException&)
    {
        //
        // The server associated to this adapter doesn't exist anymore. Somehow the database is
        // inconsistent if this happens. The best thing to do is to destroy the adapter.
        //
        destroy();
    }
    catch (const Ice::Exception& ex)
    {
        ostringstream os;
        os << "unexpected exception: " << ex;
        activationFailed(os.str());
    }
}

optional<Ice::ObjectPrx>
ServerAdapterI::getDirectProxy(const Ice::Current&) const
{
    lock_guard lock(_mutex);

    //
    // Return the adapter direct proxy if it's set. Otherwise, throw. The caller can eventually
    // activate the adapter if it's activatable.
    //
    if (_proxy && _enabled)
    {
        return _proxy;
    }
    else
    {
        throw AdapterNotActiveException(_enabled && _server->isAdapterActivatable(_id));
    }
}

void
ServerAdapterI::setDirectProxy(optional<Ice::ObjectPrx> proxy)
{
    lock_guard lock(_mutex);

    //
    // We don't allow to override an existing proxy by another non
    // null proxy if the server is not inactive.
    //
    if (!_node->allowEndpointsOverride())
    {
        if (proxy && _proxy)
        {
            if (_server->getState() == ServerState::Active)
            {
                throw AdapterActiveException();
            }
        }
    }

    bool updated = _proxy != proxy;
    _proxy = std::move(proxy);

    //
    // If the server is being deactivated and the activation callback
    // was added during the deactivation, we don't send the response
    // now. The server is going to be activated again and the adapter
    // activated.
    //
    if (_server->getState() < ServerState::Deactivating || _server->getState() >= ServerState::Destroying ||
        !_activateAfterDeactivating)
    {
        for (const auto& response : _activateCB)
        {
            response(_proxy);
        }
        _activateCB.clear();
    }

    if (updated)
    {
        _node->observerUpdateAdapter({_id, _proxy});
    }

    if (_proxy)
    {
        _server->adapterActivated(_id);
    }
    else
    {
        _server->adapterDeactivated(_id);
    }

    if (_node->getTraceLevels()->adapter > 1)
    {
        Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
        out << "server '" + _serverId + "' adapter '" << _id << "' " << (_proxy ? "activated" : "deactivated");
        if (_proxy)
        {
            out << ": " << _proxy;
        }
    }
}

void
ServerAdapterI::setDirectProxy(optional<Ice::ObjectPrx> proxy, const Ice::Current&)
{
    setDirectProxy(std::move(proxy));
}

void
ServerAdapterI::destroy()
{
    activationFailed("adapter destroyed");
    try
    {
        _node->getAdapter()->remove(_this->ice_getIdentity());
    }
    catch (const Ice::LocalException&)
    {
        // Ignore.
    }
}

void
ServerAdapterI::updateEnabled()
{
    lock_guard lock(_mutex);
    _enabled = _server->isEnabled();
}

void
ServerAdapterI::clear()
{
    lock_guard lock(_mutex);
    _proxy = nullopt;
    _activateAfterDeactivating = false;
}

void
ServerAdapterI::activationFailed(const std::string& reason)
{
    //
    // The server couldn't be activated, trace and return the current adapter proxy.
    //
    if (_node->getTraceLevels()->adapter > 1)
    {
        Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
        out << "server '" + _serverId + "' adapter '" << _id << "' activation failed: " << reason;
    }

    lock_guard lock(_mutex);
    for (const auto& response : _activateCB)
    {
        response(nullopt);
    }
    _activateCB.clear();
}

void
ServerAdapterI::activationCompleted()
{
    lock_guard lock(_mutex);
    if (!_proxy)
    {
        //
        // The server activation completed, but the adapter hasn't been activated.
        //
        if (_node->getTraceLevels()->adapter > 1)
        {
            Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->adapterCat);
            out << "server '" + _serverId + "' adapter '" << _id << "' activation failed: server activation completed";
        }
    }

    for (const auto& response : _activateCB)
    {
        response(_proxy);
    }
    _activateCB.clear();
}

AdapterPrx
ServerAdapterI::getProxy() const
{
    return _this;
}
