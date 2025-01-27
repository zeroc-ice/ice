// Copyright (c) ZeroC, Inc.

#include "ServerCache.h"
#include "AdapterCache.h"
#include "AllocatableObjectCache.h"
#include "DescriptorHelper.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "NodeCache.h"
#include "ObjectCache.h"
#include "SessionI.h"
#include "SynchronizationException.h"
#include "Topics.h"

#include "../Ice/DisableWarnings.h"

using namespace std;
using namespace IceGrid;

CheckUpdateResult::CheckUpdateResult(string server, string node, bool noRestart, bool remove, future<bool>&& result)
    : _server(std::move(server)),
      _node(std::move(node)),
      _remove(remove),
      _noRestart(noRestart),
      _result(std::move(result))
{
}

bool
CheckUpdateResult::getResult()
{
    try
    {
        return _result.get();
    }
    catch (const DeploymentException& ex)
    {
        ostringstream os;
        if (_remove)
        {
            os << "check for server '" << _server << "' remove failed: " << ex.reason;
        }
        else
        {
            os << "check for server '" << _server << "' update failed: " << ex.reason;
        }
        throw DeploymentException(os.str());
    }
    catch (const Ice::OperationNotExistException&)
    {
        if (_noRestart)
        {
            throw DeploymentException("server '" + _server + "' doesn't support check for updates");
        }
        return false;
    }
    catch (const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(_node, os.str());
    }
}

ServerCache::ServerCache(
    const shared_ptr<Ice::Communicator>& communicator,
    string instanceName,
    NodeCache& nodeCache,
    AdapterCache& adapterCache,
    ObjectCache& objectCache,
    AllocatableObjectCache& allocatableObjectCache)
    : _communicator(communicator),
      _instanceName(std::move(instanceName)),
      _nodeCache(nodeCache),
      _adapterCache(adapterCache),
      _objectCache(objectCache),
      _allocatableObjectCache(allocatableObjectCache)
{
}

shared_ptr<ServerEntry>
ServerCache::add(const ServerInfo& info)
{
    lock_guard lock(_mutex);

    auto entry = getImpl(info.descriptor->id);
    if (!entry)
    {
        entry = make_shared<ServerEntry>(*this, info.descriptor->id);
        addImpl(info.descriptor->id, entry);
    }
    entry->update(info, false);

    _nodeCache.get(info.node, true)->addServer(entry);

    forEachCommunicator(
        info.descriptor,
        [this, entry, application = info.application](const auto& descriptor)
        { addCommunicator(nullptr, descriptor, entry, application); });

    if (_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "added server '" << info.descriptor->id << "' ('" << info.uuid << "', '" << info.revision << "')";
    }

    return entry;
}

shared_ptr<ServerEntry>
ServerCache::get(const string& id) const
{
    lock_guard lock(_mutex);
    auto entry = getImpl(id);
    if (!entry)
    {
        throw ServerNotExistException(id);
    }
    return entry;
}

bool
ServerCache::has(const string& id) const
{
    lock_guard lock(_mutex);
    auto entry = getImpl(id);
    return entry && !entry->isDestroyed();
}

shared_ptr<ServerEntry>
ServerCache::remove(const string& id, bool noRestart)
{
    lock_guard lock(_mutex);

    auto entry = getImpl(id);
    assert(entry);

    ServerInfo info = entry->getInfo();
    forEachCommunicator(info.descriptor, [this, entry](const auto& desc) { removeCommunicator(desc, nullptr, entry); });
    _nodeCache.get(info.node)->removeServer(entry);

    entry->destroy(noRestart); // This must be done after otherwise some allocatable objects
                               // might allocate a destroyed server.

    if (_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "removed server '" << id << "'";
    }

    return entry;
}

void
ServerCache::preUpdate(const ServerInfo& newInfo, bool noRestart)
{
    lock_guard lock(_mutex);

    const string& id = newInfo.descriptor->id;
    auto entry = getImpl(id);
    assert(entry);

    if (!noRestart)
    {
        ServerInfo info = entry->getInfo();
        forEachCommunicator(
            info.descriptor,
            newInfo.descriptor,
            [this, entry](const auto& oldDesc, const auto& newDesc) { removeCommunicator(oldDesc, newDesc, entry); });
        _nodeCache.get(info.node)->removeServer(entry);
    }

    if (_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "updating server '" << id << "'";
        if (noRestart)
        {
            out << " with no restart";
        }
    }
}

shared_ptr<ServerEntry>
ServerCache::postUpdate(const ServerInfo& info, bool noRestart)
{
    lock_guard lock(_mutex);

    auto entry = getImpl(info.descriptor->id);
    assert(entry);

    ServerInfo oldInfo = entry->getInfo();
    entry->update(info, noRestart);

    if (!noRestart)
    {
        _nodeCache.get(info.node, true)->addServer(entry);

        forEachCommunicator(
            oldInfo.descriptor,
            info.descriptor,
            [this, entry, application = info.application](const auto& oldDesc, const auto& newDesc)
            { addCommunicator(oldDesc, newDesc, entry, application); });
    }

    if (_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "updated server '" << info.descriptor->id << "' ('" << info.uuid << "', '" << info.revision << "')";
    }

    return entry;
}

void
ServerCache::clear(const string& id)
{
    lock_guard lock(_mutex);
    CacheByString<ServerEntry>::removeImpl(id);
}

void
ServerCache::setNodeObserverTopic(const shared_ptr<NodeObserverTopic>& nodeObserverTopic)
{
    _nodeObserverTopic = nodeObserverTopic;
}

void
ServerCache::addCommunicator(
    const shared_ptr<CommunicatorDescriptor>& oldDesc,
    const shared_ptr<CommunicatorDescriptor>& newDesc,
    const shared_ptr<ServerEntry>& server,
    const string& application)
{
    if (!newDesc)
    {
        return; // Nothing to add
    }
    for (auto q = newDesc->adapters.begin(); q != newDesc->adapters.end(); ++q)
    {
        AdapterDescriptor oldAdpt;
        if (oldDesc)
        {
            for (const auto& adapter : oldDesc->adapters)
            {
                if (adapter.id == q->id)
                {
                    oldAdpt = adapter;
                    break;
                }
            }
        }
        assert(!q->id.empty());
        _adapterCache.addServerAdapter(*q, server, application);

        for (auto r = q->objects.begin(); r != q->objects.end(); ++r)
        {
            _objectCache.add(toObjectInfo(_communicator, *r, q->id), application, server->getId());
        }
        for (auto r = q->allocatables.begin(); r != q->allocatables.end(); ++r)
        {
            ObjectDescriptorSeq::const_iterator s;
            for (s = oldAdpt.allocatables.begin(); s != oldAdpt.allocatables.end() && s->id != r->id; ++s)
                ;
            if (s == oldAdpt.allocatables.end() || *s != *r) // Only add new or updated allocatables
            {
                _allocatableObjectCache.add(toObjectInfo(_communicator, *r, q->id), server);
            }
        }
    }
}

void
ServerCache::removeCommunicator(
    const shared_ptr<CommunicatorDescriptor>& oldDesc,
    const shared_ptr<CommunicatorDescriptor>& newDesc,
    const shared_ptr<ServerEntry>&)
{
    if (!oldDesc)
    {
        return; // Nothing to remove
    }
    for (auto q = oldDesc->adapters.begin(); q != oldDesc->adapters.end(); ++q)
    {
        AdapterDescriptor newAdpt;
        if (newDesc)
        {
            for (const auto& adapter : newDesc->adapters)
            {
                if (adapter.id == q->id)
                {
                    newAdpt = adapter;
                    break;
                }
            }
        }

        for (const auto& object : q->objects)
        {
            _objectCache.remove(object.id);
        }
        for (const auto& allocatable : q->allocatables)
        {
            // Don't remove the allocatable if it's still in the new descriptor.
            ObjectDescriptorSeq::const_iterator s;
            for (s = newAdpt.allocatables.begin(); s != newAdpt.allocatables.end() && s->id != allocatable.id; ++s)
                ;
            if (s == newAdpt.allocatables.end() || *s != allocatable) // Only removed updated or removed allocatables
            {
                _allocatableObjectCache.remove(allocatable.id);
            }
        }
        _adapterCache.removeServerAdapter(q->id);
    }
}

ServerEntry::ServerEntry(ServerCache& cache, string id)
    : Allocatable(false, nullptr),
      _cache(cache),
      _id(std::move(id)),
      _activationTimeout(-1),
      _deactivationTimeout(-1)
{
}

void
ServerEntry::sync()
{
    syncImpl();
}

void
ServerEntry::waitForSync(chrono::seconds timeout)
{
    waitImpl(timeout);
}

void
ServerEntry::waitForSyncNoThrow(chrono::seconds timeout)
{
    try
    {
        waitImpl(timeout);
    }
    catch (const SynchronizationException&)
    {
        assert(timeout >= 0s);
    }
    catch (const Ice::Exception&)
    {
    }
}

void
ServerEntry::unsync()
{
    lock_guard lock(_mutex);
    if (_loaded.get())
    {
        _load.reset(_loaded.release());
    }
    _proxy = nullopt;
    _adapters.clear();
    _activationTimeout = -1s;
    _deactivationTimeout = -1s;
}

bool
ServerEntry::addSyncCallback(const shared_ptr<SynchronizationCallback>& callback)
{
    lock_guard lock(_mutex);
    if (!_loaded.get() && !_load.get())
    {
        throw ServerNotExistException();
    }
    if (_synchronizing)
    {
        _callbacks.push_back(callback);
    }
    return _synchronizing;
}

void
ServerEntry::update(const ServerInfo& info, bool noRestart)
{
    lock_guard lock(_mutex);

    auto descriptor = make_unique<ServerInfo>(info);

    _updated = true;

    if (!_destroy.get())
    {
        if (_loaded.get() && descriptor->node != _loaded->node)
        {
            _destroy.reset(_loaded.release());
        }
        else if (_load.get() && descriptor->node != _load->node)
        {
            _destroy.reset(_load.release());
        }
    }

    _load.reset(descriptor.release());
    _noRestart = noRestart;
    _loaded.reset();
    _allocatable = info.descriptor->allocatable;
    if (info.descriptor->activation == "session")
    {
        _allocatable = true;
        _load->sessionId = _allocationSession ? _allocationSession->getId() : string("");
    }
}

void
ServerEntry::destroy(bool noRestart)
{
    lock_guard lock(_mutex);

    _updated = true;

    assert(_loaded.get() || _load.get());
    if (!_destroy.get())
    {
        if (_loaded.get())
        {
            assert(!_destroy.get());
            _destroy.reset(_loaded.release());
        }
        else if (_load.get())
        {
            assert(!_destroy.get());
            _destroy.reset(_load.release());
        }
    }

    _noRestart = noRestart;
    _load.reset();
    _loaded.reset();
    _allocatable = false;
}

ServerInfo
ServerEntry::getInfo(bool resolve) const
{
    ServerInfo info;
    shared_ptr<SessionI> session;
    {
        lock_guard lock(_mutex);
        if (!_loaded.get() && !_load.get())
        {
            throw ServerNotExistException();
        }
        info = _loaded.get() ? *_loaded : *_load;
        session = _allocationSession;
    }
    assert(info.descriptor);
    if (resolve)
    {
        try
        {
            return _cache.getNodeCache().get(info.node)->getServerInfo(info, session);
        }
        catch (const DeploymentException&)
        {
        }
        catch (const NodeNotExistException&)
        {
        }
        catch (const NodeUnreachableException&)
        {
        }
    }
    return info;
}

string
ServerEntry::getId() const
{
    return _id;
}

ServerPrx
ServerEntry::getProxy(bool upToDate, chrono::seconds timeout)
{
    //
    // NOTE: this might throw ServerNotExistException, NodeUnreachableException
    // or DeploymentException.
    //

    chrono::seconds actTimeout, deactTimeout;
    string node;
    return getProxy(actTimeout, deactTimeout, node, upToDate, timeout);
}

ServerPrx
ServerEntry::getProxy(
    chrono::seconds& activationTimeout,
    chrono::seconds& deactivationTimeout,
    string& node,
    bool upToDate,
    chrono::seconds timeout)
{
    //
    // NOTE: this might throw ServerNotExistException, NodeUnreachableException
    // or DeploymentException.
    //
    while (true)
    {
        {
            lock_guard lock(_mutex);
            if (_loaded || (_proxy && _synchronizing && !upToDate)) // Synced or if not up to date is fine
            {
                assert(_loaded || _load || _destroy);
                activationTimeout = _activationTimeout;
                deactivationTimeout = _deactivationTimeout;
                node = _loaded ? _loaded->node : (_load ? _load->node : _destroy->node);
                assert(_proxy);
                return *_proxy;
            }
            else if (!_load && !_destroy)
            {
                throw ServerNotExistException(_id);
            }
        }

        syncImpl();
        waitImpl(timeout);
    }
}

Ice::ObjectPrx
ServerEntry::getAdminProxy()
{
    //
    // The category must match the server admin category used by nodes
    //
    return getProxy(true)->ice_identity({_id, _cache.getInstanceName() + "-NodeServerAdminRouter"});
}

AdapterPrx
ServerEntry::getAdapter(const string& id, bool upToDate)
{
    //
    // NOTE: this might throw AdapterNotExistException, NodeUnreachableException
    // or DeploymentException.
    //

    chrono::seconds activationTimeout, deactivationTimeout;
    return getAdapter(activationTimeout, deactivationTimeout, id, upToDate);
}

AdapterPrx
ServerEntry::getAdapter(
    chrono::seconds& activationTimeout,
    chrono::seconds& deactivationTimeout,
    const string& id,
    bool upToDate)
{
    //
    // NOTE: this might throw AdapterNotExistException, NodeUnreachableException
    // or DeploymentException.
    //
    while (true)
    {
        {
            lock_guard lock(_mutex);
            if (_loaded || (_proxy && _synchronizing && !upToDate)) // Synced or if not up to date is fine
            {
                auto p = _adapters.find(id);
                if (p != _adapters.end())
                {
                    assert(p->second);
                    activationTimeout = _activationTimeout;
                    deactivationTimeout = _deactivationTimeout;
                    return *p->second;
                }
                else
                {
                    throw AdapterNotExistException(id);
                }
            }
            else if (!_load && !_destroy)
            {
                throw AdapterNotExistException(id);
            }
        }

        syncImpl();
        waitImpl(0s); // Don't wait, just check for the result or throw SynchronizationException
    }
}

float
ServerEntry::getLoad(LoadSample sample) const
{
    string application;
    string node;
    {
        lock_guard lock(_mutex);
        if (_loaded)
        {
            application = _loaded->application;
            node = _loaded->node;
        }
        else if (_load)
        {
            application = _load->application;
            node = _load->node;
        }
        else
        {
            throw ServerNotExistException();
        }
    }

    float factor;
    LoadInfo load = _cache.getNodeCache().get(node)->getLoadInfoAndLoadFactor(application, factor);
    switch (sample)
    {
        case LoadSample::LoadSample1:
            return load.avg1 < 0.f ? 1.0f : load.avg1 * factor;
        case LoadSample::LoadSample5:
            return load.avg5 < 0.f ? 1.0f : load.avg5 * factor;
        case LoadSample::LoadSample15:
            return load.avg15 < 0.f ? 1.0f : load.avg15 * factor;
        default:
            assert(false);
            return 1.0f;
    }
}

void
ServerEntry::syncImpl()
{
    ServerInfo load;
    shared_ptr<SessionI> session;
    ServerInfo destroy;
    auto timeout = -1s;
    bool noRestart = false;

    {
        lock_guard lock(_mutex);
        if (_synchronizing)
        {
            return;
        }

        if (!_load && !_destroy)
        {
            _load.reset(_loaded.release()); // Re-load the current server.
        }

        _updated = false;
        _exception = nullptr;

        if (_destroy)
        {
            destroy = *_destroy;
            timeout = _deactivationTimeout;
        }
        else if (_load)
        {
            load = *_load;
            session = _allocationSession;
            timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
        }
        else
        {
            return;
        }

        noRestart = _noRestart;
        _synchronizing = true;
    }

    if (destroy.descriptor)
    {
        try
        {
            _cache.getNodeCache()
                .get(destroy.node)
                ->destroyServer(static_pointer_cast<ServerEntry>(shared_from_this()), destroy, timeout, noRestart);
        }
        catch (const NodeNotExistException&)
        {
            exception(make_exception_ptr(NodeUnreachableException(destroy.node, "node is not active")));
        }
    }
    else if (load.descriptor)
    {
        try
        {
            _cache.getNodeCache()
                .get(load.node)
                ->loadServer(static_pointer_cast<ServerEntry>(shared_from_this()), load, session, timeout, noRestart);
        }
        catch (const NodeNotExistException&)
        {
            exception(make_exception_ptr(NodeUnreachableException(load.node, "node is not active")));
        }
    }
}

void
ServerEntry::waitImpl(chrono::seconds timeout)
{
    unique_lock lock(_mutex);
    if (timeout != 0s)
    {
        while (_synchronizing)
        {
            if (timeout > 0s)
            {
                if (_condVar.wait_for(lock, timeout) == cv_status::timeout)
                {
                    break; // Timeout
                }
            }
            else
            {
                _condVar.wait(lock);
            }
        }
    }
    if (_synchronizing) // If we are still synchronizing, throw SynchronizationException
    {
        throw SynchronizationException(__FILE__, __LINE__);
    }

    if (_exception)
    {
        try
        {
            rethrow_exception(_exception);
        }
        catch (const DeploymentException&)
        {
            throw;
        }
        catch (const NodeUnreachableException&)
        {
            throw;
        }
        catch (const Ice::Exception& ex) // This shouldn't happen.
        {
            ostringstream os;
            os << "unexpected exception while synchronizing server '" + _id + "':\n" << ex;
            auto traceLevels = _cache.getTraceLevels();
            if (traceLevels)
            {
                Ice::Error err(traceLevels->logger);
                err << os.str();
            }
            throw DeploymentException(os.str());
        }
    }
}

void
ServerEntry::synchronized()
{
    vector<shared_ptr<SynchronizationCallback>> callbacks;
    {
        lock_guard lock(_mutex);
        _callbacks.swap(callbacks);
    }
    for (const auto& callback : callbacks)
    {
        try
        {
            callback->synchronized();
        }
        catch (const std::exception&)
        {
            assert(false);
        }
    }
}

void
ServerEntry::synchronized(exception_ptr ex)
{
    vector<shared_ptr<SynchronizationCallback>> callbacks;
    {
        lock_guard lock(_mutex);
        _callbacks.swap(callbacks);
    }
    for (const auto& callback : callbacks)
    {
        try
        {
            callback->synchronized(ex);
        }
        catch (const std::exception&)
        {
            assert(false);
        }
    }
}

void
ServerEntry::loadCallback(
    ServerPrx proxy,
    const AdapterPrxDict& adapters,
    chrono::seconds activationTimeout,
    chrono::seconds deactivationTimeout)
{
    ServerInfo load;
    shared_ptr<SessionI> session;
    ServerInfo destroy;
    chrono::seconds timeout = -1s;
    bool synced = false;
    bool noRestart = false;

    {
        lock_guard lock(_mutex);
        if (!_updated)
        {
            // Set timeout on server and adapter proxies. Most of the calls on the proxies shouldn't block for longer
            // than the node session timeout. Calls that might block for a longer time should set the correct timeout
            // before invoking on the proxy (e.g.: server start/stop, adapter activate).
            assert(_load);
            _loaded.reset(_load.release());
            _proxy = std::move(proxy);
            _adapters = adapters;
            _activationTimeout = activationTimeout;
            _deactivationTimeout = deactivationTimeout;

            assert(!_destroy.get() && !_load.get());
            _synchronizing = false;
            synced = true;
            _condVar.notify_all();
        }
        else
        {
            _updated = false;
            if (_destroy.get())
            {
                destroy = *_destroy;
                noRestart = _noRestart;
            }
            else if (_load.get())
            {
                load = *_load;
                noRestart = _noRestart;
                session = _allocationSession;
                timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
            }
        }
    }

    if (synced)
    {
        synchronized();
        return;
    }

    assert(destroy.descriptor || load.descriptor);
    if (destroy.descriptor)
    {
        try
        {
            _cache.getNodeCache()
                .get(destroy.node)
                ->destroyServer(static_pointer_cast<ServerEntry>(shared_from_this()), destroy, timeout, noRestart);
        }
        catch (const NodeNotExistException&)
        {
            exception(make_exception_ptr(NodeUnreachableException(destroy.node, "node is not active")));
        }
    }
    else if (load.descriptor)
    {
        try
        {
            _cache.getNodeCache()
                .get(load.node)
                ->loadServer(static_pointer_cast<ServerEntry>(shared_from_this()), load, session, timeout, noRestart);
        }
        catch (const NodeNotExistException&)
        {
            exception(make_exception_ptr(NodeUnreachableException(load.node, "node is not active")));
        }
    }
}

void
ServerEntry::destroyCallback()
{
    ServerInfo load;
    bool noRestart = false;
    shared_ptr<SessionI> session;

    {
        lock_guard lock(_mutex);
        _destroy = nullptr;
        _proxy = nullopt;
        _adapters.clear();
        _activationTimeout = -1s;
        _deactivationTimeout = -1s;

        if (!_load.get())
        {
            assert(!_load.get() && !_loaded.get());
            _synchronizing = false;
            _condVar.notify_all();
        }
        else
        {
            _updated = false;
            load = *_load;
            noRestart = _noRestart;
            session = _allocationSession;
        }
    }

    if (load.descriptor)
    {
        try
        {
            _cache.getNodeCache()
                .get(load.node)
                ->loadServer(static_pointer_cast<ServerEntry>(shared_from_this()), load, session, -1s, noRestart);
        }
        catch (const NodeNotExistException&)
        {
            exception(make_exception_ptr(NodeUnreachableException(load.node, "node is not active")));
        }
    }
    else
    {
        synchronized();
        _cache.clear(_id);
    }
}

void
ServerEntry::exception(exception_ptr ex)
{
    ServerInfo load;
    shared_ptr<SessionI> session;
    bool noRestart = false;
    bool remove = false;
    chrono::seconds timeout = -1s;

    {
        lock_guard lock(_mutex);
        if ((_destroy.get() && !_load.get()) || (!_destroy.get() && !_updated))
        {
            remove = _destroy.get();
            _destroy = nullptr;
            _exception = ex;
            _proxy = nullopt;
            _adapters.clear();
            _activationTimeout = -1s;
            _deactivationTimeout = -1s;
            _synchronizing = false;
            _condVar.notify_all();
        }
        else
        {
            _destroy = nullptr;
            _updated = false;
            load = *_load.get();
            noRestart = _noRestart;
            session = _allocationSession;
            timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
        }
    }

    if (load.descriptor)
    {
        try
        {
            _cache.getNodeCache()
                .get(load.node)
                ->loadServer(static_pointer_cast<ServerEntry>(shared_from_this()), load, session, timeout, noRestart);
        }
        catch (const NodeNotExistException&)
        {
            exception(make_exception_ptr(NodeUnreachableException(load.node, "node is not active")));
        }
    }
    else
    {
        synchronized(ex);
        if (remove)
        {
            _cache.clear(_id);
        }
    }
}

bool
ServerEntry::isDestroyed()
{
    lock_guard lock(_mutex);
    return !_loaded.get() && !_load.get();
}

bool
ServerEntry::canRemove()
{
    lock_guard lock(_mutex);
    return !_loaded.get() && !_load.get() && !_destroy.get();
}

shared_ptr<CheckUpdateResult>
ServerEntry::checkUpdate(const ServerInfo& info, bool noRestart)
{
    shared_ptr<SessionI> session;
    ServerInfo oldInfo;
    {
        lock_guard lock(_mutex);
        if (!_loaded.get() && !_load.get())
        {
            throw ServerNotExistException();
        }

        oldInfo = _loaded.get() ? *_loaded : *_load;
        session = _allocationSession;
    }

    shared_ptr<NodeEntry> node;
    try
    {
        node = _cache.getNodeCache().get(oldInfo.node);
    }
    catch (const NodeNotExistException&)
    {
        throw NodeUnreachableException(info.node, "node is not active");
    }

    optional<ServerPrx> server;
    try
    {
        server = getProxy(true, 5s);
    }
    catch (const SynchronizationException&)
    {
        ostringstream os;
        os << "check for server '" << _id << "' update failed:";
        os << "timeout while waiting for the server to be loaded on the node";
        throw DeploymentException(os.str());
    }
    catch (const DeploymentException&)
    {
        if (noRestart)
        {
            // If the server can't be loaded and no restart is required, we throw to indicate that the server update
            // can't be checked.
            throw;
        }
        else
        {
            // Otherwise, we do as if the update is valid.
            return nullptr;
        }
    }

    // Provide a null descriptor if the server is to be removed from the node. In this case, the check just ensures
    // that the server is stopped.
    shared_ptr<InternalServerDescriptor> desc;
    if (info.node == oldInfo.node && info.descriptor)
    {
        desc = node->getInternalServerDescriptor(info, session); // The new descriptor
    }

    assert(server);
    return make_shared<CheckUpdateResult>(
        _id,
        oldInfo.node,
        noRestart,
        desc != nullptr,
        server->checkUpdateAsync(desc, noRestart));
}

bool
ServerEntry::isEnabled() const
{
    return _cache.getNodeObserverTopic()->isServerEnabled(_id);
}

void
ServerEntry::allocated(const shared_ptr<SessionI>& session)
{
    if (!_loaded.get() && !_load.get())
    {
        return;
    }

    auto traceLevels = _cache.getTraceLevels();
    if (traceLevels && traceLevels->server > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
        out << "server '" << _id << "' allocated by '" << session->getId() << "' (" << _count << ")";
    }

    auto desc = _loaded.get() ? _loaded->descriptor : _load->descriptor;

    //
    // If the server has the session activation mode, we re-load the
    // server on the node as its deployment might have changed (it's
    // possible to use ${session.*} variable with server with the
    // session activation mode.
    //
    if (desc->activation == "session")
    {
        _updated = true;
        if (!_load.get())
        {
            _load.reset(_loaded.release());
        }
        _allocationSession = session;
        _load->sessionId = session->getId();
    }

    auto identitySet = session->getGlacier2IdentitySet();
    auto adapterIdSet = session->getGlacier2AdapterIdSet();
    if (identitySet && adapterIdSet)
    {
        auto helper = createHelper(desc);
        multiset<string> adapterIds;
        multiset<Ice::Identity> identities;
        helper->getIds(adapterIds, identities);
        try
        {
            adapterIdSet->add({adapterIds.begin(), adapterIds.end()});
            identitySet->add({identities.begin(), identities.end()});
        }
        catch (const Ice::LocalException& ex)
        {
            if (traceLevels && traceLevels->server > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                out << "couldn't add Glacier2 filters for server '" << _id << "' allocated by '" << session->getId()
                    << ":\n"
                    << ex;
            }
        }
    }
}

void
ServerEntry::allocatedNoSync(const shared_ptr<SessionI>&)
{
    {
        lock_guard lock(_mutex);
        if (!_updated || (_loaded.get() && _loaded->descriptor->activation != "session") ||
            (_load.get() && _load->descriptor->activation != "session"))
        {
            return;
        }
    }

    sync();
    waitForSyncNoThrow();
}

void
ServerEntry::released(const shared_ptr<SessionI>& session)
{
    if (!_loaded.get() && !_load.get())
    {
        return;
    }

    auto desc = _loaded.get() ? _loaded->descriptor : _load->descriptor;

    //
    // If the server has the session activation mode, we re-load the
    // server on the node as its deployment might have changed (it's
    // possible to use ${session.*} variable with server with the
    // session activation mode. Synchronizing the server will also
    // shutdown the server on the node.
    //
    if (desc->activation == "session")
    {
        _updated = true;
        if (!_load.get())
        {
            _load.reset(_loaded.release());
        }
        _load->sessionId = "";
        _allocationSession = nullptr;
    }

    auto traceLevels = _cache.getTraceLevels();

    auto identitySet = session->getGlacier2IdentitySet();
    auto adapterIdSet = session->getGlacier2AdapterIdSet();
    if (identitySet && adapterIdSet)
    {
        auto helper = createHelper(desc);
        multiset<string> adapterIds;
        multiset<Ice::Identity> identities;
        helper->getIds(adapterIds, identities);
        try
        {
            adapterIdSet->remove({adapterIds.begin(), adapterIds.end()});
            identitySet->remove({identities.begin(), identities.end()});
        }
        catch (const Ice::LocalException& ex)
        {
            if (traceLevels && traceLevels->server > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                out << "couldn't remove Glacier2 filters for server '" << _id << "' allocated by '";
                out << session->getId() << ":\n" << ex;
            }
        }
    }

    if (traceLevels && traceLevels->server > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
        out << "server '" << _id << "' released by '" << session->getId() << "' (" << _count << ")";
    }
}

void
ServerEntry::releasedNoSync(const shared_ptr<SessionI>& /*session*/)
{
    {
        lock_guard lock(_mutex);
        if (!_updated || (_loaded.get() && _loaded->descriptor->activation != "session") ||
            (_load.get() && _load->descriptor->activation != "session"))
        {
            return;
        }
    }

    sync();
    waitForSyncNoThrow();
}
