// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>

#include <IceGrid/ServerCache.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/AdapterCache.h>
#include <IceGrid/ObjectCache.h>

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

ServerCache::ServerCache(NodeCache& nodeCache, 
			 AdapterCache& adapterCache, 
			 ObjectCache& objectCache) :
    _nodeCache(nodeCache), 
    _adapterCache(adapterCache), 
    _objectCache(objectCache)
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

    if(_traceLevels && _traceLevels->server > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	out << "added server `" << info.descriptor->id << "'";	
    }

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

    if(destroy)
    {
	if(_traceLevels && _traceLevels->server > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "removed server `" << id << "'";	
	}
    }

    return entry;
}

void
ServerCache::clear(const string& id)
{
    Lock sync(*this);
    CacheByString<ServerEntry>::removeImpl(id);
}

NodeCache&
ServerCache::getNodeCache() const
{
    return _nodeCache;
}

void
ServerCache::addCommunicator(const CommunicatorDescriptorPtr& comm, const ServerEntryPtr& entry)
{
    const string application = entry->getApplication();
    for(AdapterDescriptorSeq::const_iterator q = comm->adapters.begin() ; q != comm->adapters.end(); ++q)
    {
	if(!q->id.empty())
	{
	    _adapterCache.get(q->id, true)->addServer(entry);
	}
	for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
	{
	    const string edpts = IceGrid::getProperty(comm->properties, q->name + ".Endpoints");
	    _objectCache.add(application, q->id, edpts, *r);
	}
    }
}

void
ServerCache::removeCommunicator(const CommunicatorDescriptorPtr& comm, const ServerEntryPtr& entry)
{
    for(AdapterDescriptorSeq::const_iterator q = comm->adapters.begin() ; q != comm->adapters.end(); ++q)
    {
	if(!q->id.empty())
	{
	    _adapterCache.get(q->id)->removeServer(entry);
	}
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
    string node;
    try
    {
	syncImpl(adapters, at, dt, node);
    }
    catch(const NodeUnreachableException&)
    {
    }
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
ServerEntry::getProxy(int& activationTimeout, int& deactivationTimeout, string& node)
{
    ServerPrx proxy;
    {
	Lock sync(*this);
	if(_proxy) // Synced
	{
	    proxy = _proxy;
	    activationTimeout = _activationTimeout;
	    deactivationTimeout = _deactivationTimeout;
	    node = _loaded->node;
	}
    }

    if(proxy)
    {
	try
	{
	    proxy->ice_ping();
	    return proxy;
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    StringAdapterPrxDict adapters;
    proxy = syncImpl(adapters, activationTimeout, deactivationTimeout, node);
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
	catch(const Ice::LocalException&)
	{
	}
    }

    StringAdapterPrxDict adapters;
    int activationTimeout, deactivationTimeout;
    string node;
    syncImpl(adapters, activationTimeout, deactivationTimeout, node);
    AdapterPrx adapter = adapters[id];
    if(!adapter)
    {
	throw AdapterNotExistException();
    }
    return adapter;
}

NodeEntryPtr
ServerEntry::getNode() const
{
    Lock sync(*this);
    if(!_loaded.get() && !_load.get())
    {
	throw ServerNotExistException();
    }
    return _proxy ? _cache.getNodeCache().get(_loaded->node) : _cache.getNodeCache().get(_load->node);
}

string
ServerEntry::getApplication() const
{
    Lock sync(*this);
    if(!_loaded.get() && !_load.get())
    {
	throw ServerNotExistException();
    }
    return _proxy ? _loaded->application : _load->application;
}

float
ServerEntry::getLoad(LoadSample sample) const
{
    Lock sync(*this);
    if(!_loaded.get() && !_load.get())
    {
	throw ServerNotExistException();
    }

    ServerInfo& info = _proxy ? *_loaded.get() : *_load.get();
    float factor;
    LoadInfo load = _cache.getNodeCache().get(info.node)->getLoadInfoAndLoadFactor(info.application, factor);
    if(factor < 0.0f)
    {
	factor = 1.0f / load.nProcessors;
    }
    switch(sample)
    {
    case LoadSample1:
	return load.load1 * factor;
    case LoadSample5:
	return load.load5 * factor;
    case LoadSample15:
	return load.load15 * factor;
    default:
	assert(false);
	return 1.0f;
    }
}

ServerPrx
ServerEntry::syncImpl(map<string, AdapterPrx>& adpts, int& activationTimeout, int& deactivationTimeout, string& node)
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
	    _proxy = 0;
	    _adapters.clear();
	    _activationTimeout = 0;
	    _deactivationTimeout = 0;
	}

	_synchronizing = true;
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
    NodeCache& nodeCache = _cache.getNodeCache();
    try
    {
	if(destroy)
	{
	    try
	    {
		nodeCache.get(destroyNode)->getProxy()->destroyServer(destroy->id);

		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "unloaded server `" << destroy->id << "' on node `" << destroyNode << "'";	
		}
	    }
	    catch(const NodeNotExistException& ex)
	    {
		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "couldn't unload server `" << destroy->id << "' on node `" << destroyNode << "':\n" << ex;
		}

		if(!load)
		{
		    ostringstream os;
		    os << ex;
		    throw NodeUnreachableException(destroyNode, os.str());
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "couldn't unload server `" << destroy->id << "' on node `" << destroyNode << "':\n" << ex;
		}

		if(!load)
		{
		    ostringstream os;
		    os << ex;
		    throw NodeUnreachableException(destroyNode, os.str());
		}
	    }
	}

	if(load)
	{
	    try
	    {
		const NodePrx n = nodeCache.get(loadNode)->getProxy();
		proxy = n->loadServer(load, adpts, activationTimeout, deactivationTimeout);
		node = loadNode;
		proxy = ServerPrx::uncheckedCast(proxy->ice_collocationOptimization(false));

		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "loaded server `" << load->id << "' on node `" << loadNode << "'";	
		}
	    }
	    catch(const NodeNotExistException& ex)
	    {
		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "couldn't load server `" << load->id << "' on node `" << loadNode << "':\n" << ex;
		}

		ostringstream os;
		os << ex;
		throw NodeUnreachableException(loadNode, os.str());
	    }
	    catch(const DeploymentException& ex)
	    {
		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "couldn't load server `" << load->id << "' on node `" << loadNode << "':\n" << ex.reason;
		}

		Ice::Warning out(_cache.getTraceLevels()->logger);
		out << "failed to load server on node `" << loadNode << "':\n" << ex;
		ostringstream os;
		os << ex << "\nreason: " << ex.reason;
		throw NodeUnreachableException(loadNode, os.str());
	    }
	    catch(const Ice::Exception& ex)
	    {
		if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 1)
		{
		    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
		    out << "couldn't load server `" << load->id << "' on node `" << loadNode << "':\n" << ex;
		}

		ostringstream os;
		os << ex;
		throw NodeUnreachableException(loadNode, os.str());
	    }
	}
    }
    catch(const NodeUnreachableException&)
    {
	{
	    Lock sync(*this);
	    _synchronizing = false;
	    _destroy.reset(0);
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
	    int timeout = nodeCache.getSessionTimeout() * 1000; // sec to ms
	    _loaded = _load;
	    assert(_loaded.get());
	    _proxy = ServerPrx::uncheckedCast(proxy->ice_timeout(timeout));
	    _adapters.clear();
	    for(StringAdapterPrxDict::const_iterator p = adpts.begin(); p != adpts.end(); ++p)
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
