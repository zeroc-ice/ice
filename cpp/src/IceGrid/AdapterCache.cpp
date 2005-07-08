// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/AdapterCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>

using namespace std;
using namespace IceGrid;

AdapterEntryPtr
AdapterCache::get(const string& id, bool create) const
{
    Lock sync(*this);
    AdapterCache& self = const_cast<AdapterCache&>(*this);
    AdapterEntryPtr entry = self.getImpl(id, create);
    if(!entry)
    {
	AdapterNotExistException ex;
	ex.id = id;
	throw ex;
    }
    return entry;
}

AdapterEntry::AdapterEntry(Cache<string, AdapterEntry>& cache, const std::string& id) : 
    _cache(cache),
    _id(id),
    _replicated(false)
{
}

void
AdapterEntry::enableReplication(LoadBalancingPolicy policy)
{
    Lock sync(*this);
    _replicated = true;
    _loadBalancing = policy;
}

void
AdapterEntry::disableReplication()
{
    _replicated = false;
}

void
AdapterEntry::addServer(const ServerEntryPtr& entry)
{
    Lock sync(*this);
    assert(_replicated || _servers.empty());
    _servers.insert(make_pair(entry->getName(), entry));
}

void
AdapterEntry::removeServer(const ServerEntryPtr& entry)
{
    bool remove = false;
    {
	Lock sync(*this);
	_servers.erase(entry->getName());
	remove = _servers.empty();
    }
    if(remove)
    {
	_cache.remove(_id);
    }    
}

AdapterPrx
AdapterEntry::getProxy(const string& serverId) const
{
    Lock sync(*this);
    if(!_replicated)
    {
	return _servers.begin()->second->getAdapter(_id);
    }
    else
    {
	//
	// TODO: implement load balancing strategies. This is also not really correct to use the first adapter 
	// if the server id is empty this could  allow a server to always override the first server endpoints.
	//
	map<string, ServerEntryPtr>::const_iterator p = serverId.empty() ? _servers.begin() : _servers.find(serverId);
	if(p == _servers.end())
	{
	    AdapterNotExistException ex;
	    ex.id = _id;
	    throw ex;
	}
	return p->second->getAdapter(_id);
    }
}

bool
AdapterEntry::canRemove()
{
    Lock sync(*this);
    return _servers.empty();
}
