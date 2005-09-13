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
#include <IceGrid/NodeCache.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

struct ServerLoadCI : binary_function<ServerEntryPtr&, ServerEntryPtr&, bool>
{
    ServerLoadCI(LoadSample loadSample) : _loadSample(loadSample) { }

    bool operator()(const ServerEntryPtr& lhs, const ServerEntryPtr& rhs)
    {
	float lhsl = 1.0f;
	try
	{
	    lhsl = lhs->getLoad(_loadSample);
	}
	catch(const ServerNotExistException&)
	{
	}
	catch(const NodeUnreachableException&)
	{
	}

	float rhsl = 1.0f;
	try
	{
	    rhsl = rhs->getLoad(_loadSample);
	}
	catch(const ServerNotExistException&)
	{
	}
	catch(const NodeUnreachableException&)
	{
	}
	return lhsl < rhsl;
    }

    LoadSample _loadSample;
};

}

AdapterCache::AdapterCache(const TraceLevelsPtr& traceLevels) : CacheByString<AdapterEntry>(traceLevels)
{
}

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
AdapterEntry::enableReplication(const LoadBalancingPolicyPtr& policy)
{
    {
	Lock sync(*this);
	_replicated = true;
	_loadBalancing = policy;
	istringstream is(policy->nReplicas);
	is >> _loadBalancingNReplicas;
	if(_loadBalancingNReplicas < 1)
	{
	    _loadBalancingNReplicas = 1;
	}
	else if(_loadBalancingNReplicas > static_cast<int>(_servers.size()))
	{
	    _loadBalancingNReplicas = _servers.size();
	}
	AdaptiveLoadBalancingPolicyPtr alb = AdaptiveLoadBalancingPolicyPtr::dynamicCast(_loadBalancing);
	if(alb)
	{
	    if(alb->loadSample == "1")
	    {
		_loadSample = LoadSample1;
	    }
	    else if(alb->loadSample == "5")
	    {
		_loadSample = LoadSample5;
	    }
	    else if(alb->loadSample == "15")
	    {
		_loadSample = LoadSample15;
	    }
	    else
	    {
		_loadSample = LoadSample1;
	    }
	}
    }
}

void
AdapterEntry::disableReplication()
{
    bool remove;
    {
	Lock sync(*this);
	_replicated = false;
	remove = _servers.empty();
    }
    if(remove)
    {
	_cache.remove(_id);
    }
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
	remove = _servers.empty() && !_replicated;
    }
    if(remove)
    {
	_cache.remove(_id);
    }
}

vector<pair<string, AdapterPrx> >
AdapterEntry::getProxies(int& nReplicas)
{
    vector<ServerEntryPtr> servers;
    bool adaptive = false;
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
	    if(RoundRobinLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	    {
		for(unsigned int i = 0; i < _servers.size(); ++i)
		{
		    servers.push_back(_servers[(_lastServer + i) % _servers.size()]);
		}
		_lastServer = (_lastServer + 1) % _servers.size();
	    }
	    else if(AdaptiveLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	    {
		servers = _servers;
		adaptive = true;
	    }
	    else// if(RandomLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	    {
		servers = _servers;
		random_shuffle(servers.begin(), servers.end());
	    }
	}
    }

    if(adaptive)
    {
	//
	// This must be done outside the synchronization block since
	// the sort() will call and lock each server entry.
	//
	random_shuffle(servers.begin(), servers.end());
	sort(servers.begin(), servers.end(), ServerLoadCI(_loadSample));
    }

    vector<pair<string, AdapterPrx> > adapters;
    for(vector<ServerEntryPtr>::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	try
	{
	    adapters.push_back(make_pair((*p)->getId(), (*p)->getAdapter(_id)));
	}
	catch(const NodeUnreachableException&)
	{
	}
    }
    if(adapters.empty())
    {
	throw NodeUnreachableException();
    }

    nReplicas = _replicated ? _loadBalancingNReplicas : 1;
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
		if((*p)->getId() == serverId) // getId() doesn't lock the server so it's safe.
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
	ex.id = serverId;
	throw ex;
    }

    return server->getAdapter(_id);
}

bool
AdapterEntry::canRemove()
{
    Lock sync(*this);
    return _servers.empty() && !_replicated;
}
