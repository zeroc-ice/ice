// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
            _serverCache.addCommunicator(desc, _entry, _application);
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
            _serverCache.removeCommunicator(desc, _entry);
        }

        ServerCache& _serverCache;
        const ServerEntryPtr _entry;
    };
}

ServerCache::ServerCache(const Ice::CommunicatorPtr& communicator,
                         NodeCache& nodeCache, 
                         AdapterCache& adapterCache, 
                         ObjectCache& objectCache,
                         AllocatableObjectCache& allocatableObjectCache) :
    _communicator(communicator),
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
    entry->update(info);
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
ServerCache::remove(const string& id, bool destroy)
{
    Lock sync(*this);

    ServerEntryPtr entry = getImpl(id);
    ServerInfo info = entry->getInfo();
    forEachCommunicator(RemoveCommunicator(*this, entry))(info.descriptor);

    _nodeCache.get(info.node)->removeServer(entry);

    if(destroy)
    {
        entry->destroy(); // This must be done after otherwise some allocatable objects
                          // might allocate a destroyed server.
    }

    if(_traceLevels && _traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "removed server `" << id << "'"; 
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
ServerCache::addCommunicator(const CommunicatorDescriptorPtr& comm, 
                             const ServerEntryPtr& server,
                             const string& application)
{
    for(AdapterDescriptorSeq::const_iterator q = comm->adapters.begin() ; q != comm->adapters.end(); ++q)
    {
        assert(!q->id.empty());
        _adapterCache.addServerAdapter(*q, server, application);

        ObjectDescriptorSeq::const_iterator r;
        for(r = q->objects.begin(); r != q->objects.end(); ++r)
        {
            ObjectInfo info;
            info.type = r->type;
            info.proxy = _communicator->stringToProxy("\"" + _communicator->identityToString(r->id) + "\" @ " + q->id);
            _objectCache.add(info, application);
        }

        for(r = q->allocatables.begin(); r != q->allocatables.end(); ++r)
        {
            ObjectInfo info;
            info.type = r->type;
            info.proxy = _communicator->stringToProxy("\"" + _communicator->identityToString(r->id) + "\" @ " + q->id);
            _allocatableObjectCache.add(info, server);
        }
    }
}

void
ServerCache::removeCommunicator(const CommunicatorDescriptorPtr& comm, const ServerEntryPtr& entry)
{
    for(AdapterDescriptorSeq::const_iterator q = comm->adapters.begin() ; q != comm->adapters.end(); ++q)
    {
        ObjectDescriptorSeq::const_iterator r;
        for(r = q->objects.begin(); r != q->objects.end(); ++r)
        {
            _objectCache.remove(r->id);
        }
        for(r = q->allocatables.begin(); r != q->allocatables.end(); ++r)
        {
            _allocatableObjectCache.remove(r->id);
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
    _updated(false)
{
}

void
ServerEntry::sync()
{
    syncImpl();
}

void
ServerEntry::syncAndWait()
{
    syncImpl();
    try
    {
        waitImpl();
    }
    catch(const NodeUnreachableException&)
    {
        //
        // The node being unreachable isn't considered as a failure to
        // synchronize the server.
        //
    }
}

void
ServerEntry::waitNoThrow()
{
    try
    {
        waitImpl();
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
        _load = _loaded;
    }
    _proxy = 0;
    _adapters.clear();
    _activationTimeout = -1;
    _deactivationTimeout = -1;
}

void
ServerEntry::update(const ServerInfo& info)
{
    Lock sync(*this);

    auto_ptr<ServerInfo> descriptor(new ServerInfo());
    *descriptor = info;

    _updated = true;

    if(!_destroy.get())
    {
        if(_loaded.get() && descriptor->node != _loaded->node)
        {
            _destroy = _loaded;
        }
        else if(_load.get() && descriptor->node != _load->node)
        {
            _destroy = _load;
        }
    }

    _load = descriptor;
    _loaded.reset(0);
    _allocatable = info.descriptor->allocatable || info.descriptor->activation == "session";
}

void
ServerEntry::destroy()
{
    Lock sync(*this);

    _updated = true;

    assert(_loaded.get() || _load.get());
    if(!_destroy.get())
    {
        if(_loaded.get())
        {
            assert(!_destroy.get());
            _destroy = _loaded;
        }
        else if(_load.get())
        {
            assert(!_destroy.get());
            _destroy = _load;
        }
    }
    
    _load.reset(0);
    _loaded.reset(0);
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
ServerEntry::getProxy(bool upToDate)
{
    int actTimeout, deactTimeout;
    string node;
    return getProxy(actTimeout, deactTimeout, node, upToDate);
}

ServerPrx
ServerEntry::getProxy(int& activationTimeout, int& deactivationTimeout, string& node, bool upToDate)
{
    {
        Lock sync(*this);
        if(_loaded.get() || _proxy && _synchronizing && !upToDate) // Synced or if not up to date is fine
        {
            assert(_loaded.get() || _load.get() || _destroy.get());
            activationTimeout = _activationTimeout;
            deactivationTimeout = _deactivationTimeout;
            node = _loaded.get() ? _loaded->node : (_load.get() ? _load->node : _destroy->node);
            return _proxy;
        }
    }

    while(true)
    {
        //
        // Note that we don't call syncAndWait() because we want
        // NodeUnreachableException exceptions to go through.
        //
        syncImpl();
        waitImpl();

        {
            Lock sync(*this);
            if(_loaded.get() || _proxy && _synchronizing && !upToDate) // Synced or if not up to date is fine
            {
                assert(_loaded.get() || _load.get() || _destroy.get());
                activationTimeout = _activationTimeout;
                deactivationTimeout = _deactivationTimeout;
                node = _loaded.get() ? _loaded->node : (_load.get() ? _load->node : _destroy->node);
                return _proxy;
            }
            else if(_load.get())
            {
                continue; // Retry
            }
            else
            {
                throw ServerNotExistException(_id);
            }
        }
    }
}

AdapterPrx
ServerEntry::getAdapter(const string& id, bool upToDate)
{
    int activationTimeout, deactivationTimeout;
    return getAdapter(activationTimeout, deactivationTimeout, id, upToDate);
}

AdapterPrx
ServerEntry::getAdapter(int& activationTimeout, int& deactivationTimeout, const string& id, bool upToDate)
{
    {
        Lock sync(*this);
        if(_loaded.get() || _proxy && _synchronizing && !upToDate) // Synced or if not up to date is fine
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
    }

    while(true)
    {    
        //
        // Note that we don't call syncAndWait() because we want
        // NodeUnreachableException exceptions to go through.
        //
        syncImpl();
        waitImpl();

        {
            Lock sync(*this);
            if(_loaded.get() || _proxy && _synchronizing && !upToDate) // Synced or if not up to date is fine
            {
                AdapterPrxDict::const_iterator p = _adapters.find(id);
                if(p != _adapters.end())
                {
                    activationTimeout = _activationTimeout;
                    deactivationTimeout = _deactivationTimeout;
                    return p->second;
                }
                else
                {
                    throw AdapterNotExistException(id);
                }
            }
            else if(_load.get())
            {
                continue; // Retry
            }
            else
            {
                throw AdapterNotExistException(id);                 
            }
        }
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

    {
        Lock sync(*this);
        if(_synchronizing)
        {
            return;
        }

        if(!_load.get() && !_destroy.get())
        {
            _load = _loaded; // Re-load the current server.
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

        _synchronizing = true;
    }
    
    if(destroy.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(destroy.node)->destroyServer(this, destroy, timeout);
        }
        catch(NodeNotExistException&)
        {
            exception(NodeUnreachableException(destroy.node, "node is not active"));
        }    
    }
    else if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, timeout);
        }
        catch(NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }    
}

void
ServerEntry::waitImpl()
{
    Lock sync(*this);
    while(_synchronizing)
    {
        wait();
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
ServerEntry::loadCallback(const ServerPrx& proxy, const AdapterPrxDict& adpts, int at, int dt)
{
    ServerInfo load;
    SessionIPtr session;
    ServerInfo destroy;
    int timeout = -1;

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
            _loaded = _load;
            assert(_loaded.get());
            _proxy = proxy;
            _adapters = adpts;
            _activationTimeout = at;
            _deactivationTimeout = dt;

            assert(!_destroy.get() && !_load.get());
            _synchronizing = false;
            notifyAll();
            return;
        }
        else
        {
            _updated = false;
            if(_destroy.get())
            {
                destroy = *_destroy;
            }
            else if(_load.get())
            {
                load = *_load;
                session = _session;
                timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
            }
        }
    }

    assert(destroy.descriptor || load.descriptor);
    if(destroy.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(destroy.node)->destroyServer(this, destroy, timeout);
        }
        catch(NodeNotExistException&)
        {
            exception(NodeUnreachableException(destroy.node, "node is not active"));
        }    
    }
    else if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, timeout);
        }
        catch(NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
}

void
ServerEntry::destroyCallback()
{
    ServerInfo load;
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
            session = _session;
        }
    }

    if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, -1);
        }
        catch(NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
    else
    {
        _cache.clear(_id);
    }
}

void
ServerEntry::exception(const Ice::Exception& ex)
{
    ServerInfo load;
    SessionIPtr session;
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
            session = _session;
            timeout = _deactivationTimeout; // loadServer might block to deactivate the previous server.
        }
    }

    if(load.descriptor)
    {
        try
        {
            _cache.getNodeCache().get(load.node)->loadServer(this, load, session, timeout);
        }
        catch(NodeNotExistException&)
        {
            exception(NodeUnreachableException(load.node, "node is not active"));
        }
    }
    else if(remove)
    {
        _cache.clear(_id);
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
            _load = _loaded;
        }
        _session = session;
        _load->sessionId = session->getId();
    }

    Glacier2::SessionControlPrx ctl = session->getSessionControl();
    if(ctl)
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
            ctl->adapterIds()->add(adapterIdSeq);
            ctl->identities()->add(identitySeq);
        }
        catch(const Ice::LocalException& ex)
        {
            if(traceLevels && traceLevels->server > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->serverCat);
                out << "couldn't add Glacier2 filters for server `" << _id << "' allocated by `" << session->getId();
                out << ":\n" << ex;
            }
        }
    }
}

void
ServerEntry::allocatedNoSync(const SessionIPtr& session)
{
    {
        Lock sync(*this);
        if(!_updated || 
           _loaded.get() && _loaded->descriptor->activation != "session" || 
           _load.get() && _load->descriptor->activation != "session")
        {
            return;
        }
    }
    
    sync();
    waitNoThrow();
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
            _load = _loaded;
        }
        _load->sessionId = "";
        _session = 0;
    }

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();

    Glacier2::SessionControlPrx ctl = session->getSessionControl();
    if(ctl)
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
            ctl->adapterIds()->remove(adapterIdSeq);
            ctl->identities()->remove(identitySeq);
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
ServerEntry::releasedNoSync(const SessionIPtr& session)
{
    {
        Lock sync(*this);
        if(!_updated || 
           _loaded.get() && _loaded->descriptor->activation != "session" || 
           _load.get() && _load->descriptor->activation != "session")
        {
            return;
        }
    }

    sync();
    waitNoThrow();
}
