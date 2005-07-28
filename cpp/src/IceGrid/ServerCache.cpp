// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/ServerCache.h>
#include <IceGrid/Database.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

    struct AddCommunicator : std::unary_function<CommunicatorDescriptorPtr&, void>
    {
	AddCommunicator(ServerCache& serverCache, const ServerEntryPtr& entry) :
	    _serverCache(serverCache), _entry(entry)
	{
	}
	
	void
	operator()(const CommunicatorDescriptorPtr& desc)
	{
	    _serverCache.addCommunicator(desc, _entry);
	}
	
	ServerCache& _serverCache;
	const ServerEntryPtr _entry;
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

ServerCache::ServerCache(Database& db, NodeCache& nodeCache, AdapterCache& adapterCache, ObjectCache& objectCache) :
    _database(db), _nodeCache(nodeCache), _adapterCache(adapterCache), _objectCache(objectCache)
{
}

ServerEntryPtr
ServerCache::add(const ServerInfo& info)
{
    Lock sync(*this);

    ServerEntryPtr entry = getImpl(info.descriptor->id, true);
    entry->update(info);
    _nodeCache.get(info.node, true)->addServer(entry);

    forEachCommunicator(AddCommunicator(*this, entry))(info.descriptor);
    return entry;
}

ServerEntryPtr
ServerCache::get(const string& id)
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

ServerEntryPtr
ServerCache::remove(const string& id, bool destroy)
{
    Lock sync(*this);

    ServerEntryPtr entry = getImpl(id);
    ServerInfo info = entry->getServerInfo();
    if(destroy)
    {
	entry->destroy();
    }

    _nodeCache.get(info.node)->removeServer(entry);

    forEachCommunicator(RemoveCommunicator(*this, entry))(info.descriptor);

    return entry;
}

void
ServerCache::clear(const string& id)
{
    Lock sync(*this);
    CacheByString<ServerEntry>::removeImpl(id);
}

Database&
ServerCache::getDatabase() const
{
    return _database;
}

void
ServerCache::addCommunicator(const CommunicatorDescriptorPtr& comm, const ServerEntryPtr& entry)
{
    for(AdapterDescriptorSeq::const_iterator q = comm->adapters.begin() ; q != comm->adapters.end(); ++q)
    {
	_adapterCache.get(q->id, true)->addServer(entry);
	for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
	{
	    _objectCache.add(q->id, *r);
	}
    }
}

void
ServerCache::removeCommunicator(const CommunicatorDescriptorPtr& comm, const ServerEntryPtr& entry)
{
    for(AdapterDescriptorSeq::const_iterator q = comm->adapters.begin() ; q != comm->adapters.end(); ++q)
    {
	_adapterCache.get(q->id)->removeServer(entry);
	for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
	{
	    _objectCache.remove(r->id);
	}
    }
}

ServerEntry::ServerEntry(Cache<string, ServerEntry>& cache, const string& id) :
    _cache(*dynamic_cast<ServerCache*>(&cache)), 
    _id(id),
    _synchronizing(false)
{
}

void
ServerEntry::sync()
{
    map<string, AdapterPrx> adapters;
    int at, dt;
    try
    {
	sync(adapters, at, dt);
    }
    catch(const NodeUnreachableException&)
    {
    }
}

bool
ServerEntry::needsSync() const
{
    Lock sync(*this);
    return _failed;
}

void
ServerEntry::update(const ServerInfo& info)
{
    Lock sync(*this);

    auto_ptr<ServerInfo> descriptor(new ServerInfo());
    *descriptor = info;

    if(_loaded.get() && descriptor->node != _loaded->node)
    {
	assert(!_destroy.get());
	_destroy = _loaded;
    }
    else if(_load.get() && descriptor->node != _load->node)
    {
	assert(!_destroy.get());
	_destroy = _load;
    }

    _load = descriptor;
    _loaded.reset(0);
    _proxy = 0;
    _adapters.clear();
}

void
ServerEntry::destroy()
{
    Lock sync(*this);

    assert(_loaded.get() || _load.get());
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

    _load.reset(0);
    _loaded.reset(0);
    _proxy = 0;
    _adapters.clear();
}

ServerInfo
ServerEntry::getServerInfo() const
{
    Lock sync(*this);
    if(!_loaded.get() && !_load.get())
    {
	throw ServerNotExistException();
    }
    return _proxy ? *_loaded : *_load;
}

string
ServerEntry::getId() const
{
    return _id;
}

ServerPrx
ServerEntry::getProxy(int& activationTimeout, int& deactivationTimeout)
{
    ServerPrx proxy;
    {
	Lock sync(*this);
	if(_proxy) // Synced
	{
	    proxy = _proxy;
	    activationTimeout = _activationTimeout;
	    deactivationTimeout = _deactivationTimeout;
	}
    }

    if(proxy)
    {
	try
	{
	    proxy->ice_ping();
	    return proxy;
	}
	catch(const Ice::LocalException& ex)
	{
	}
    }

    StringAdapterPrxDict adapters;
    proxy = sync(adapters, activationTimeout, deactivationTimeout);
    if(!proxy)
    {
	throw ServerNotExistException();
    }
    return proxy;
}

AdapterPrx
ServerEntry::getAdapter(const string& id)
{
    AdapterPrx proxy;
    {
	Lock sync(*this);
	if(_proxy) // Synced
	{
	    proxy = _adapters[id];
	}
    }

    if(proxy)
    {
	try
	{
	    proxy->ice_ping();
	    return proxy;
	}
	catch(const Ice::LocalException& ex)
	{
	}
    }

    StringAdapterPrxDict adapters;
    int activationTimeout, deactivationTimeout;
    sync(adapters, activationTimeout, deactivationTimeout);
    AdapterPrx adapter = adapters[id];
    if(!adapter)
    {
	throw AdapterNotExistException();
    }
    return adapter;
}

ServerPrx
ServerEntry::sync(map<string, AdapterPrx>& adapters, int& activationTimeout, int& deactivationTimeout)
{
    ServerDescriptorPtr load;
    string loadNode;
    ServerDescriptorPtr destroy;
    string destroyNode;
    {
	Lock sync(*this);
	while(_synchronizing)
	{
	    wait();
	}

	if(!_load.get() && !_destroy.get())
	{
	    _load = _loaded; // Re-load the current server.
	}

	_synchronizing = true;
	_failed = false;
	if(_load.get())
	{
	    load = _load->descriptor;
	    loadNode = _load->node;
	}
	if(_destroy.get())
	{
	    destroy = _destroy->descriptor;
	    destroyNode = _destroy->node;
	}
    }

    ServerPrx proxy;
    Database& db = _cache.getDatabase();
    try
    {
	if(destroy)
	{
	    try
	    {
		db.getNode(destroyNode)->destroyServer(destroy->id);
	    }
	    catch(const NodeNotExistException& ex)
	    {
		if(!load)
		{
		    throw NodeUnreachableException();
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(!load)
		{
		    throw NodeUnreachableException();
		}
	    }
	}

	if(load)
	{
	    try
	    {
		proxy = db.getNode(loadNode)->loadServer(load, adapters, activationTimeout, deactivationTimeout);
		proxy = ServerPrx::uncheckedCast(proxy->ice_collocationOptimization(false));
	    }
	    catch(const NodeNotExistException& ex)
	    {
		throw NodeUnreachableException();
	    }
	    catch(const DeploymentException& ex)
	    {
		Ice::Warning out(db.getTraceLevels()->logger);
		out << "failed to load server on node `" << loadNode << "':\n" << ex;
		throw NodeUnreachableException();
	    }
	    catch(const Ice::LocalException& ex)
	    {
		Ice::Warning out(db.getTraceLevels()->logger);
		out << "unexpected exception while loading on node `" << loadNode << "':\n" << ex;
		throw NodeUnreachableException();
	    }
	}
    }
    catch(const NodeUnreachableException& ex)
    {
	{
	    Lock sync(*this);
	    _synchronizing = false;
	    _destroy.reset(0);
	    _failed = true;
	    notifyAll();
	}
	if(!load && destroy)
	{
	    _cache.clear(destroy->id);
	}
	throw;
    }

    {
	Lock sync(*this);
	_synchronizing = false;
	_loaded = _load;
	_load.reset(0);
	_destroy.reset(0);

	//
	// Set timeout on server and adapter proxies. Most of the
	// calls on the proxies shouldn't block for longer than the
	// node session timeout. Calls that might block for a longer
	// time should set the correct timeout before invoking on the
	// proxy (e.g.: server start/stop, adapter activate).
	//
	if(proxy)
	{
	    int timeout = db.getNodeSessionTimeout() * 1000; // sec to ms
	    _proxy = ServerPrx::uncheckedCast(proxy->ice_timeout(timeout));
	    _adapters.clear();
	    for(StringAdapterPrxDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
	    {
		AdapterPrx adapter = AdapterPrx::uncheckedCast(p->second->ice_timeout(timeout));
		_adapters.insert(make_pair(p->first, adapter));
	    }
	    activationTimeout += timeout;
	    deactivationTimeout += timeout;
	    _activationTimeout = activationTimeout;
	    _deactivationTimeout = deactivationTimeout;
	}
	else
	{
	    _proxy = 0;
	    _adapters.clear();
	    _activationTimeout = 0;
	    _deactivationTimeout = 0;
	}
	notifyAll();
    }
    if(!load && destroy)
    {
	_cache.clear(destroy->id);
    }
    return proxy;
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
