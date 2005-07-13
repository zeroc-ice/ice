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
    _replicated(false),
    _lastServer(0)
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
    _servers.push_back(entry);
}

void
AdapterEntry::removeServer(const ServerEntryPtr& entry)
{
    bool remove = false;
    {
	Lock sync(*this);
	for(ServerEntrySeq::iterator p = _servers.begin(); p != _servers.end(); ++p)
	{
	    if(entry.get() == p->get())
	    {
		_servers.erase(p);
		break;
	    }
	}
	remove = _servers.empty();
    }
    if(remove)
    {
	_cache.remove(_id);
    }    
}

vector<pair<string, AdapterPrx> >
AdapterEntry::getProxies(int& endpointCount)
{
    vector<ServerEntryPtr> servers;
    {
	Lock sync(*this);	
	if(_servers.empty())
	{
	    AdapterNotExistException ex;
	    ex.id = _id;
	    throw ex;
	}

	if(!_replicated)
	{
	    servers.push_back(_servers[0]);
	}
	else
	{
	    servers.reserve(_servers.size());
	    switch(_loadBalancing)
	    {
	    case Random:
		servers = _servers;
		random_shuffle(servers.begin(), servers.end());
		break;
	    case RoundRobin:
		for(unsigned int i = 0; i < _servers.size(); ++i)
		{
		    servers.push_back(_servers[(_lastServer + i) % _servers.size()]);
		}
		_lastServer = (_lastServer + 1) % _servers.size();
		break;
	    case Adaptive:
		servers = _servers; // TODO
		break;
	    }
	}
    }

    vector<pair<string, AdapterPrx> > adapters;
    for(vector<ServerEntryPtr>::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	try
	{
	    adapters.push_back(make_pair((*p)->getName(), (*p)->getAdapter(_id)));
	}
	catch(const NodeUnreachableException&)
	{
	}
    }
    if(adapters.empty())
    {
	throw NodeUnreachableException();
    }
    endpointCount = 1;
    return adapters;
}

AdapterPrx
AdapterEntry::getProxy(const string& serverId) const
{
    ServerEntryPtr server;
    {
	Lock sync(*this);
	if(_servers.empty())
	{
	    AdapterNotExistException ex;
	    ex.id = _id;
	    throw ex;
	}

	if(!_replicated)
	{
	    server = _servers[0];
	}
	else
	{
	    for(ServerEntrySeq::const_iterator p = _servers.begin(); p != _servers.end(); ++p)
	    {
		if((*p)->getName() == serverId)
		{
		    server = *p;
		    break;
		}
	    }
	}
    }

    if(!server)
    {
	ServerNotExistException ex;
	ex.name = serverId;
	throw ex;
    }

    return server->getAdapter(_id);
}

bool
AdapterEntry::canRemove()
{
    Lock sync(*this);
    return _servers.empty();
}
