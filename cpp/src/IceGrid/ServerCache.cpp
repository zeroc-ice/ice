// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/AdapterCache.h>
#include <IceGrid/ObjectCache.h>
#include <IceGrid/AllocatableObjectCache.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/DescriptorHelper.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

    struct AddCommunicator : std::unary_function<CommunicatorDescriptorPtr&, void>
    {
        AddCommunicator(ServerCache& serverCache, const ServerEntryPtr& entry, const string& application) :
            _serverCache(serverCache), _entry(entry), _application(application)
        {
        }

        void
        operator()(const CommunicatorDescriptorPtr& desc)
        {
            _serverCache.addCommunicator(0, desc, _entry, _application);
        }

        void
        operator()(const CommunicatorDescriptorPtr& oldDesc, const CommunicatorDescriptorPtr& newDesc)
        {
            _serverCache.addCommunicator(oldDesc, newDesc, _entry, _application);
        }

        ServerCache& _serverCache;
        const ServerEntryPtr _entry;
        const string _application;
    };

    struct RemoveCommunicator : std::unary_function<CommunicatorDescriptorPtr&, void>
    {
        RemoveCommunicator(ServerCache& serverCache, const ServerEntryPtr& entry) :
            _serverCache(serverCache), _entry(entry)
        {
        }

        void
        operator()(const CommunicatorDescriptorPtr& desc)
        {
            _serverCache.removeCommunicator(desc, 0, _entry);
        }

        void
        operator()(const CommunicatorDescriptorPtr& oldDesc, const CommunicatorDescriptorPtr& newDesc)
        {
            _serverCache.removeCommunicator(oldDesc, newDesc, _entry);
        }

        ServerCache& _serverCache;
        const ServerEntryPtr _entry;
    };


}

CheckUpdateResult::CheckUpdateResult(const string& server,
                                     const string& node,
                                     bool noRestart,
                                     bool remove,
                                     const Ice::AsyncResultPtr& result) :
    _server(server), _node(node), _noRestart(noRestart), _result(result)
{
}

bool
CheckUpdateResult::getResult()
{
    try
    {
        return ServerPrx::uncheckedCast(_result->getProxy())->end_checkUpdate(_result);
    }
    catch(const DeploymentException& ex)
    {
        ostringstream os;
        os << "check for server `" << _server << "' update failed: " << ex.reason;
        throw DeploymentException(os.str());
    }
    catch(const Ice::OperationNotExistException&)
    {
        if(_noRestart)
        {
            throw DeploymentException("server `" + _server + "' doesn't support check for updates");
        }
        return false;
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream os;
        os << ex;
        throw NodeUnreachableException(_node, os.str());
    }
    return false;
}

ServerCache::ServerCache(const Ice::CommunicatorPtr& communicator,
                         const string& instanceName,
                         NodeCache& nodeCache,
                         AdapterCache& adapterCache,
                         ObjectCache& objectCache,
                         AllocatableObjectCache& allocatableObjectCache) :
    _communicator(communicator),
    _instanceName(instanceName),
    _nodeCache(nodeCache),
    _adapterCache(adapterCache),
    _objectCache(objectCache),
    _allocatableObjectCache(allocatableObjectCache)
{
}

ServerEntryPtr
ServerCache::add(const ServerInfo& info)
{
    Lock sync(*this);

    ServerEntryPtr entry = getImpl(info.descriptor->id);
    if(!entry)
    {
        entry = new ServerEntry(*this, info.descriptor->id);
        addImpl(info.descriptor->id, entry);
    }
    entry->update(info, false);

    _nodeCache.get(info.node, true)->addServer(entry);

    forEachCommunicator(AddCommunicator(*this, entry, info.application))(info.descriptor);

    if(_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "added server `" << info.descriptor->id << "' (`" << info.uuid << "', `" << info.revision << "')";
    }

    return entry;
}

ServerEntryPtr
ServerCache::get(const string& id) const
{
    Lock sync(*this);
    ServerEntryPtr entry = getImpl(id);
    if(!entry)
    {
        ServerNotExistException ex;
        ex.id = id;
        throw ex;
    }
    return entry;
}

bool
ServerCache::has(const string& id) const
{
    Lock sync(*this);
    ServerEntryPtr entry = getImpl(id);
    return entry && !entry->isDestroyed();
}

ServerEntryPtr
ServerCache::remove(const string& id, bool noRestart)
{
    Lock sync(*this);

    ServerEntryPtr entry = getImpl(id);
    assert(entry);

    ServerInfo info = entry->getInfo();
    forEachCommunicator(RemoveCommunicator(*this, entry))(info.descriptor);

    _nodeCache.get(info.node)->removeServer(entry);

    entry->destroy(noRestart); // This must be done after otherwise some allocatable objects
                               // might allocate a destroyed server.

    if(_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "removed server `" << id << "'";
    }

    return entry;
}

void
ServerCache::preUpdate(const ServerInfo& newInfo, bool noRestart)
{
    Lock sync(*this);

    const string& id = newInfo.descriptor->id;
    ServerEntryPtr entry = getImpl(id);
    assert(entry);

    if(!noRestart)
    {
        ServerInfo info = entry->getInfo();
        forEachCommunicator(RemoveCommunicator(*this, entry))(info.descriptor, newInfo.descriptor);
        _nodeCache.get(info.node)->removeServer(entry);
    }

    if(_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "updating server `" << id << "'";
        if(noRestart)
        {
            out << " with no restart";
        }
    }
}

ServerEntryPtr
ServerCache::postUpdate(const ServerInfo& info, bool noRestart)
{
    Lock sync(*this);

    ServerEntryPtr entry = getImpl(info.descriptor->id);
    assert(entry);

    ServerInfo oldInfo = entry->getInfo();
    entry->update(info, noRestart);

    if(!noRestart)
    {
        _nodeCache.get(info.node, true)->addServer(entry);
        forEachCommunicator(AddCommunicator(*this, entry, info.application))(oldInfo.descriptor, info.descriptor);
    }

    if(_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "updated server `" << info.descriptor->id << "' (`" << info.uuid << "', `" << info.revision << "')";
    }

    return entry;
}

void
ServerCache::clear(const string& id)
{
    Lock sync(*this);
    CacheByString<ServerEntry>::removeImpl(id);
}

void
ServerCache::addCommunicator(const CommunicatorDescriptorPtr& oldDesc,
                             const CommunicatorDescriptorPtr& newDesc,
                             const ServerEntryPtr& server,
                             const string& application)
{
    if(!newDesc)
    {
        return; // Nothing to add
    }
    for(AdapterDescriptorSeq::const_iterator q = newDesc->adapters.begin() ; q != newDesc->adapters.end(); ++q)
    {
        AdapterDescriptor oldAdpt;
        if(oldDesc)
        {
            for(AdapterDescriptorSeq::const_iterator p = oldDesc->adapters.begin() ; p != oldDesc->adapters.end(); ++p)
            {
                if(p->id == q->id)
                {
                    oldAdpt = *p;
                    break;
                }
            }
        }
        assert(!q->id.empty());
        _adapterCache.addServerAdapter(*q, server, application);

        for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
        {
            _objectCache.add(toObjectInfo(_communicator, *r, q->id), application);
        }
        for(ObjectDescriptorSeq::const_iterator r = q->allocatables.begin(); r != q->allocatables.end(); ++r)
        {
            ObjectDescriptorSeq::const_iterator s;
            for(s = oldAdpt.allocatables.begin(); s != oldAdpt.allocatables.end() && s->id != r->id; ++s);
            if(s == oldAdpt.allocatables.end() || *s != *r) // Only add new or updated allocatables
            {
                _allocatableObjectCache.add(toObjectInfo(_communicator, *r, q->id), server);
            }
        }
    }
}

void
ServerCache::removeCommunicator(const CommunicatorDescriptorPtr& oldDesc,
                                const CommunicatorDescriptorPtr& newDesc,
                                const ServerEntryPtr& /*entry*/)
{
    if(!oldDesc)
    {
        return; // Nothing to remove
    }
    for(AdapterDescriptorSeq::const_iterator q = oldDesc->adapters.begin() ; q != oldDesc->adapters.end(); ++q)
    {
        AdapterDescriptor newAdpt;
        if(newDesc)
        {
            for(AdapterDescriptorSeq::const_iterator p = newDesc->adapters.begin() ; p != newDesc->adapters.end(); ++p)
            {
                if(p->id == q->id)
                {
                    newAdpt = *p;
                    break;
                }
            }
        }

        for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
        {
            _objectCache.remove((*r).id);
        }
        for(ObjectDescriptorSeq::const_iterator r = q->allocatables.begin(); r != q->allocatables.end(); ++r)
        {
            // Don't remove the allocatable if it's still in the new descriptor.
            ObjectDescriptorSeq::const_iterator s;
            for(s = newAdpt.allocatables.begin(); s != newAdpt.allocatables.end() && s->id != r->id; ++s);
            if(s == newAdpt.allocatables.end() || *s != *r) // Only removed updated or removed allocatables
            {
                _allocatableObjectCache.remove(r->id);
            }
        }
        _adapterCache.removeServerAdapter(q->id);
    }
}

ServerEntry::ServerEntry(ServerCache& cache, const string& id) :
    Allocatable(false, 0),
    _cache(cache),
    _id(id),
    _activationTimeout(-1),
    _deactivationTimeout(-1),
    _synchronizing(false),
    _updated(false),
    _noRestart(false)
{
}

void
ServerEntry::sync()
{
    syncImpl();
}

void
ServerEntry::waitForSync(int timeout)
{
    waitImpl(timeout);
}

void
ServerEntry::waitForSyncNoThrow(int timeout)
{
    try
    {
        waitImpl(timeout);
    }
    catch(const SynchronizationException&)
    {
        assert(timeout >= 0);
    }
    catch(const Ice::Exception&)
    {
    }
}

void
ServerEntry::unsync()
{
    Lock sync(*this);
    if(_loaded.get())
    {
        _load.reset(_loaded.release());
    }
    _proxy = 0;
    _adapters.clear();
    _activationTimeout = -1;
    _deactivationTimeout = -1;
}

bool
ServerEntry::addSyncCallback(const SynchronizationCallbackPtr& callback)
{
    Lock sync(*this);
    if(!_loaded.get() && !_load.get())
    {
        throw ServerNotExistException();
    }
    if(_synchronizing)
    {
        _callbacks.push_back(callback);
    }
    return _synchronizing;
}

void
ServerEntry::update(const ServerInfo& info, bool noRestart)
{
    Lock sync(*this);

    IceInternal::UniquePtr<ServerInfo> descriptor(new ServerInfo());
    *descriptor = info;

    _updated = true;

    if(!_destroy.get())
    {
        if(_loaded.get() && descriptor->node != _loaded->node)
        {
            _destroy.reset(_loaded.release());
        }
        else if(_load.get() && descriptor->node != _load->node)
        {
            _destroy.reset(_load.release());
        }
    }

    _load.reset(descriptor.release());
    _noRestart = noRestart;
    _loaded.reset();
    _allocatable = info.descriptor->allocatable;
    if(info.descriptor->activation == "session")
    {
        _allocatable = true;
        _load->sessionId = _session ? _session->getId() : string("");
    }
}

void
ServerEntry::destroy(bool noRestart)
{
    Lock sync(*this);

    _updated = true;

    assert(_loaded.get() || _load.get());
    if(!_destroy.get())
    {
        if(_loaded.get())
        {
            assert(!_destroy.get());
            _destroy.reset(_loaded.release());
        }
        else if(_load.get())
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
    SessionIPtr session;
    {
        Lock sync(*this);
        if(!_loaded.get() && !_load.get())
        {
            throw ServerNotExistException();
        }
        info = _loaded.get() ? *_loaded : *_load;
        session = _session;
    }
    assert(info.descriptor);
    if(resolve)
    {
        try
        {
            return _cache.getNodeCache().get(info.node)->getServerInfo(info, session);
        }
        catch(const DeploymentException&)
        {
        }
        catch(const NodeNotExistException&)
        {
        }
        catch(const NodeUnreachableException&)
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
ServerEntry::getProxy(bool upToDate, int timeout)
{
    //
    // NOTE: this might throw ServerNotExistException, NodeUnreachableException
    // or DeploymentException.
    //

    int actTimeout, deactTimeout;
    string node;
    return getProxy(actTimeout, deactTimeout, node, upToDate, timeout);
}

ServerPrx
ServerEntry::getProxy(int& activationTimeout, int& deactivationTimeout, string& node, bool upToDate, int timeout)
{
    //
    // NOTE: this might throw ServerNotExistException, NodeUnreachableException
    // or DeploymentException.
    //
    while(true)
    {
        {
            Lock sync(*this);
            if(_loaded.get() || (_proxy && _synchronizing && !upToDate)) // Synced or if not up to date is fine
            {
                assert(_loaded.get() || _load.get() || _destroy.get());
                activationTimeout = _activationTimeout;
                deactivationTimeout = _deactivationTimeout;
                node = _loaded.get() ? _loaded->node : (_load.get() ? _load->node : _destroy->node);
                return _proxy;
            }
            else if(!_load.get() && !_destroy.get())
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
    Ice::Identity adminId;
    adminId.name = _id;
    adminId.category = _cache.getInstanceName() + "-NodeServerAdminRouter";
    return getProxy(true)->ice_identity(adminId);
}

AdapterPrx
ServerEntry::getAdapter(const string& id, bool upToDate)
{
    //
    // NOTE: this might throw AdapterNotExistException, NodeUnreachableException
    // or DeploymentException.
    //

    int activationTimeout, deactivationTimeout;
    return getAdapter(activationTimeout, deactivationTimeout, id, upToDate);
}

AdapterPrx
ServerEntry::getAdapter(int& activationTimeout, int& deactivationTimeout, const string& id, bool upToDate)
{
    //
    // NOTE: this might throw AdapterNotExistException, NodeUnreachableException
    // or DeploymentException.
    //
    while(true)
    {
        {
            Lock sync(*this);
            if(_loaded.get() || (_proxy && _synchronizing && !upToDate)) // Synced or if not up to date is fine
            {
                AdapterPrxDict::const_iterator p = _adapters.find(id);
                if(p != _adapters.end())
                {
                    assert(p->second);
                    activationTimeout = _activationTimeout;
                    deactivationTimeout = _deactivationTimeout;
                    return p->second;
                }
                else
                {
                    throw AdapterNotExistException(id);
                }
            }
            else if(!_load.get() && !_destroy.get())
            {
                throw AdapterNotExistException(id);
            }
        }

        syncImpl();
        waitImpl(0); // Don't wait, just check for the result or throw SynchronizationException
    }
}

float
ServerEntry::getLoad(LoadSample sample) const
{
    string application;
    string node;
    {
        Lock sync(*this);
        if(_loaded.get())
        {
            application = _loaded->application;
            node = _loaded->node;
        }
        else if(_load.get())
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
    switch(sample)
    {
    case LoadSample1:
        return load.avg1 < 0.f ? 1.0f : load.avg1 * factor;
    case LoadSample5:
        return load.avg5 < 0.f ? 1.0f : load.avg5 * factor;
    case LoadSample15:
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
    SessionIPtr session;
    ServerInfo destroy;
    int timeout = -1;
    bool noRestart = false;

    {
        Lock sync(*this);
        if(_synchronizing)
        {
            return;
        }

        if(!_load.get() && !_destroy.get())
        {
            _load.reset(_loaded.release()); // Re-load the current server.
        }

        _updated = false;
        _exception.reset(0);

        if(_destroy.get())
        {
            destroy = *_destroy;
            timeout = _deactivationTimeout;
        }
        else if(_load.get())
        {
            load = *_load;
            session = _session;
            timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
        }
        else
        {
            return;
        }

        noRestart = _noRestart;
        _synchronizing = true;
    }

    if(destroy.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(destroy.node)->destroyServer(this, destroy, timeout, noRestart);
        }
        catch(const NodeNotExistException&)
        {
            exception(NodeUnreachableException(destroy.node, "node is not active"));
        }
    }
    else if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, timeout, noRestart);
        }
        catch(const NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
}

void
ServerEntry::waitImpl(int timeout)
{
    Lock sync(*this);
    if(timeout != 0)
    {
        while(_synchronizing)
        {
            if(timeout > 0)
            {
                if(!timedWait(IceUtil::Time::seconds(timeout)))
                {
                    break; // Timeout
                }
            }
            else
            {
                wait();
            }
        }
    }
    if(_synchronizing) // If we are still synchronizing, throw SynchronizationException
    {
        throw SynchronizationException(__FILE__, __LINE__);
    }

    if(_exception.get())
    {
        try
        {
            _exception->ice_throw();
        }
        catch(const DeploymentException&)
        {
            throw;
        }
        catch(const NodeUnreachableException&)
        {
            throw;
        }
        catch(const Ice::Exception& ex) // This shouln't happen.
        {
            ostringstream os;
            os << "unexpected exception while synchronizing server `" + _id + "':\n" << ex;
            TraceLevelsPtr traceLevels = _cache.getTraceLevels();
            if(traceLevels)
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
    vector<SynchronizationCallbackPtr> callbacks;
    {
        Lock sync(*this);
        _callbacks.swap(callbacks);
    }
    for(vector<SynchronizationCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        try
        {
            (*p)->synchronized();
        }
        catch(...)
        {
            assert(false);
        }
    }
}

void
ServerEntry::synchronized(const Ice::Exception& ex)
{
    vector<SynchronizationCallbackPtr> callbacks;
    {
        Lock sync(*this);
        _callbacks.swap(callbacks);
    }
    for(vector<SynchronizationCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        try
        {
            (*p)->synchronized(ex);
        }
        catch(...)
        {
            assert(false);
        }
    }
}

void
ServerEntry::loadCallback(const ServerPrx& proxy, const AdapterPrxDict& adpts, int at, int dt)
{
    ServerInfo load;
    SessionIPtr session;
    ServerInfo destroy;
    int timeout = -1;
    bool synced = false;
    bool noRestart = false;

    {
        Lock sync(*this);
        if(!_updated)
        {
            //
            // Set timeout on server and adapter proxies. Most of the
            // calls on the proxies shouldn't block for longer than the
            // node session timeout. Calls that might block for a longer
            // time should set the correct timeout before invoking on the
            // proxy (e.g.: server start/stop, adapter activate).
            //
            assert(_load.get());
            _loaded.reset(_load.release());
            _proxy = proxy;
            _adapters = adpts;
            _activationTimeout = at;
            _deactivationTimeout = dt;

            assert(!_destroy.get() && !_load.get());
            _synchronizing = false;
            synced = true;
            notifyAll();
        }
        else
        {
            _updated = false;
            if(_destroy.get())
            {
                destroy = *_destroy;
                noRestart = _noRestart;
            }
            else if(_load.get())
            {
                load = *_load;
                noRestart = _noRestart;
                session = _session;
                timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
            }
        }
    }

    if(synced)
    {
        synchronized();
        return;
    }

    assert(destroy.descriptor || load.descriptor);
    if(destroy.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(destroy.node)->destroyServer(this, destroy, timeout, noRestart);
        }
        catch(const NodeNotExistException&)
        {
            exception(NodeUnreachableException(destroy.node, "node is not active"));
        }
    }
    else if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, timeout, noRestart);
        }
        catch(const NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
}

void
ServerEntry::destroyCallback()
{
    ServerInfo load;
    bool noRestart = false;
    SessionIPtr session;

    {
        Lock sync(*this);
        _destroy.reset(0);
        _proxy = 0;
        _adapters.clear();
        _activationTimeout = -1;
        _deactivationTimeout = -1;

        if(!_load.get())
        {
            assert(!_load.get() && !_loaded.get());
            _synchronizing = false;
            notifyAll();
        }
        else
        {
            _updated = false;
            load = *_load;
            noRestart = _noRestart;
            session = _session;
        }
    }

    if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, -1, noRestart);
        }
        catch(const NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
    else
    {
        synchronized();
        _cache.clear(_id);
    }
}

void
ServerEntry::exception(const Ice::Exception& ex)
{
    ServerInfo load;
    SessionIPtr session;
    bool noRestart = false;
    bool remove = false;
    int timeout = -1;

    {
        Lock sync(*this);
        if((_destroy.get() && !_load.get()) || (!_destroy.get() && !_updated))
        {
            remove = _destroy.get();
            _destroy.reset(0);
            _exception.reset(ex.ice_clone());
            _proxy = 0;
            _adapters.clear();
            _activationTimeout = -1;
            _deactivationTimeout = -1;
            _synchronizing = false;
            notifyAll();
        }
        else
        {
            _destroy.reset(0);
            _updated = false;
            load = *_load.get();
            noRestart = _noRestart;
            session = _session;
            timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
        }
    }

    if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, timeout, noRestart);
        }
        catch(const NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
    else
    {
        synchronized(ex);
        if(remove)
        {
            _cache.clear(_id);
        }
    }
}

bool
ServerEntry::isDestroyed()
{
     Lock sync(*this);
     return !_loaded.get() && !_load.get();
}

bool
ServerEntry::canRemove()
{
     Lock sync(*this);
     return !_loaded.get() && !_load.get() && !_destroy.get();
}

CheckUpdateResultPtr
ServerEntry::checkUpdate(const ServerInfo& info, bool noRestart)
{
    SessionIPtr session;
    ServerInfo oldInfo;
    {
        Lock sync(*this);
        if(!_loaded.get() && !_load.get())
        {
            throw ServerNotExistException();
        }

        oldInfo = _loaded.get() ? *_loaded : *_load;
        session = _session;
    }

    NodeEntryPtr node;
    try
    {
        node = _cache.getNodeCache().get(oldInfo.node);
    }
    catch(const NodeNotExistException&)
    {
        throw NodeUnreachableException(info.node, "node is not active");
    }

    ServerPrx server;
    try
    {
        server = getProxy(true, 5);
    }
    catch(const SynchronizationException&)
    {
        ostringstream os;
        os << "check for server `" << _id << "' update failed:";
        os << "timeout while waiting for the server to be loaded on the node";
        throw DeploymentException(os.str());
    }
    catch(const DeploymentException&)
    {
        if(noRestart)
        {
            // If the server can't be loaded and no restart is required, we throw
            // to indicate that the server update can't be checked.
            throw;
        }
        else
        {
            // Otherwise, we do as if the update is valid.
            return 0;
        }
    }

    //
    // Provide a null descriptor if the server is to be removed from
    // the node. In this case, the check just ensures that the server
    // is stopped.
    //
    InternalServerDescriptorPtr desc;
    if(info.node == oldInfo.node && info.descriptor)
    {
        desc = node->getInternalServerDescriptor(info, session); // The new descriptor
    }

    return new CheckUpdateResult(_id, oldInfo.node, noRestart, desc, server->begin_checkUpdate(desc, noRestart));
}

void
ServerEntry::allocated(const SessionIPtr& session)
{
    if(!_loaded.get() && !_load.get())
    {
        return;
    }

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->server > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
        out << "server `" << _id << "' allocated by `" << session->getId() << "' (" << _count << ")";
    }

    ServerDescriptorPtr desc = _loaded.get() ? _loaded->descriptor : _load->descriptor;

    //
    // If the server has the session activation mode, we re-load the
    // server on the node as its deployment might have changed (it's
    // possible to use ${session.*} variable with server with the
    // session activation mode.
    //
    if(desc->activation == "session")
    {
        _updated = true;
        if(!_load.get())
        {
            _load.reset(_loaded.release());
        }
        _session = session;
        _load->sessionId = session->getId();
    }


    Glacier2::IdentitySetPrx identitySet = session->getGlacier2IdentitySet();
    Glacier2::StringSetPrx adapterIdSet = session->getGlacier2AdapterIdSet();
    if(identitySet && adapterIdSet)
    {
        ServerHelperPtr helper = createHelper(desc);
        multiset<string> adapterIds;
        multiset<Ice::Identity> identities;
        helper->getIds(adapterIds, identities);
        try
        {
            //
            // SunCC won't accept the following:
            //
            // ctl->adapterIds()->add(Ice::StringSeq(adapterIds.begin(), adapterIds.end()));
            // ctl->identities()->add(Ice::IdentitySeq(identities.begin(), identities.end()));
            //
            Ice::StringSeq adapterIdSeq;
            for(multiset<string>::iterator p = adapterIds.begin(); p != adapterIds.end(); ++p)
            {
                adapterIdSeq.push_back(*p);
            }
            Ice::IdentitySeq identitySeq;
            for(multiset<Ice::Identity>::iterator q = identities.begin(); q != identities.end(); ++q)
            {
                identitySeq.push_back(*q);
            }
            adapterIdSet->add(adapterIdSeq);
            identitySet->add(identitySeq);
        }
        catch(const Ice::LocalException& ex)
        {
            if(traceLevels && traceLevels->server > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                out << "couldn't add Glacier2 filters for server `" << _id << "' allocated by `"
                    << session->getId() << ":\n" << ex;
            }
        }
    }
}

void
ServerEntry::allocatedNoSync(const SessionIPtr& /*session*/)
{
    {
        Lock sync(*this);
        if(!_updated ||
           (_loaded.get() && _loaded->descriptor->activation != "session") ||
           (_load.get() && _load->descriptor->activation != "session"))
        {
            return;
        }
    }

    sync();
    waitForSyncNoThrow();
}

void
ServerEntry::released(const SessionIPtr& session)
{
    if(!_loaded.get() && !_load.get())
    {
        return;
    }

    ServerDescriptorPtr desc = _loaded.get() ? _loaded->descriptor : _load->descriptor;

    //
    // If the server has the session activation mode, we re-load the
    // server on the node as its deployment might have changed (it's
    // possible to use ${session.*} variable with server with the
    // session activation mode. Synchronizing the server will also
    // shutdown the server on the node.
    //
    if(desc->activation == "session")
    {
        _updated = true;
        if(!_load.get())
        {
            _load.reset(_loaded.release());
        }
        _load->sessionId = "";
        _session = 0;
    }

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();

    Glacier2::IdentitySetPrx identitySet = session->getGlacier2IdentitySet();
    Glacier2::StringSetPrx adapterIdSet = session->getGlacier2AdapterIdSet();
    if(identitySet && adapterIdSet)
    {
        ServerHelperPtr helper = createHelper(desc);
        multiset<string> adapterIds;
        multiset<Ice::Identity> identities;
        helper->getIds(adapterIds, identities);
        try
        {
            //
            // SunCC won't accept the following:
            //
            // ctl->adapterIds()->remove(Ice::StringSeq(adapterIds.begin(), adapterIds.end()));
            // ctl->identities()->remove(Ice::IdentitySeq(identities.begin(), identities.end()));
            //
            Ice::StringSeq adapterIdSeq;
            for(multiset<string>::iterator p = adapterIds.begin(); p != adapterIds.end(); ++p)
            {
                adapterIdSeq.push_back(*p);
            }
            Ice::IdentitySeq identitySeq;
            for(multiset<Ice::Identity>::iterator q = identities.begin(); q != identities.end(); ++q)
            {
                identitySeq.push_back(*q);
            }
            adapterIdSet->remove(adapterIdSeq);
            identitySet->remove(identitySeq);
        }
        catch(const Ice::LocalException& ex)
        {
            if(traceLevels && traceLevels->server > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                out << "couldn't remove Glacier2 filters for server `" << _id << "' allocated by `";
                out << session->getId() << ":\n" << ex;
            }
        }
    }

    if(traceLevels && traceLevels->server > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
        out << "server `" << _id << "' released by `" << session->getId() << "' (" << _count << ")";
    }
}

void
ServerEntry::releasedNoSync(const SessionIPtr& /*session*/)
{
    {
        Lock sync(*this);
        if(!_updated ||
           (_loaded.get() && _loaded->descriptor->activation != "session") ||
           (_load.get() && _load->descriptor->activation != "session"))
        {
            return;
        }
    }

    sync();
    waitForSyncNoThrow();
}
