// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>

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

    bool operator()(const pair<string, ServerEntryPtr>& lhs, const pair<string, ServerEntryPtr>& rhs)
    {
	float lhsl = 1.0f;
	try
	{
	    lhsl = lhs.second->getLoad(_loadSample);
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
	    rhsl = rhs.second->getLoad(_loadSample);
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

AdapterEntryPtr
AdapterCache::get(const string& id, bool create) const
{
    Lock sync(*this);
    AdapterCache& self = const_cast<AdapterCache&>(*this);
    AdapterEntryPtr entry = self.getImpl(id, create);
    if(!entry)
    {
	throw AdapterNotExistException(id, "");
    }
    return entry;
}

AdapterEntryPtr
AdapterCache::addImpl(const string& id)
{
    if(_traceLevels && _traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "added adapter `" << id << "'";	
    }    
    return Cache<string, AdapterEntry>::addImpl(id);
}

AdapterEntryPtr
AdapterCache::removeImpl(const string& id)
{
    if(_traceLevels && _traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "removed adapter `" << id << "'";	
    }    
    return Cache<string, AdapterEntry>::removeImpl(id);
}

AdapterEntry::AdapterEntry(Cache<string, AdapterEntry>& cache, const std::string& id) : 
    _cache(cache),
    _id(id),
    _replicated(false),
    _lastReplica(0)
{
}

void
AdapterEntry::enableReplication(const LoadBalancingPolicyPtr& policy)
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
    
    if(_cache.getTraceLevels() && _cache.getTraceLevels()->adapter > 0)
    {
	Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->adapterCat);
	out << "enabled replication on adapter `" << _id << "'";	
    }
}

void
AdapterEntry::disableReplication()
{
    bool remove;
    {
	Lock sync(*this);
	_replicated = false;
	remove = _replicas.empty();
    }
    if(_cache.getTraceLevels() && _cache.getTraceLevels()->adapter > 0)
    {
	Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->adapterCat);
	out << "disabled replication on adapter `" << _id << "'";	
    }
    if(remove)
    {
	_cache.remove(_id);
    }
}

void
AdapterEntry::addReplica(const string& replicaId, const ServerEntryPtr& entry)
{
    Lock sync(*this);
    assert(_replicated || _replicas.empty());
    _replicas.push_back(make_pair(replicaId, entry));
}

void
AdapterEntry::removeReplica(const string& replicaId)
{
    bool remove = false;
    {
	Lock sync(*this);
	for(ReplicaSeq::iterator p = _replicas.begin(); p != _replicas.end(); ++p)
	{
	    if(replicaId == p->first)
	    {
		_replicas.erase(p);		
		 // Make sure _lastReplica is still within the bounds.
		_lastReplica = _replicas.empty() ? 0 : _lastReplica % _replicas.size();
		break;
	    }
	}
	remove = _replicas.empty() && !_replicated;
    }
    if(remove)
    {
	_cache.remove(_id);
    }
}

vector<pair<string, AdapterPrx> >
AdapterEntry::getProxies(int& nReplicas)
{
    ReplicaSeq replicas;
    bool adaptive = false;
    LoadSample loadSample;
    {
	Lock sync(*this);	
	if(_replicas.empty())
	{
	    throw AdapterNotExistException(_id, "");
	}

	if(!_replicated)
	{
	    nReplicas = 1;
	    replicas.push_back(_replicas[0]);
	}
	else
	{
	    nReplicas = _loadBalancingNReplicas;

	    replicas.reserve(_replicas.size());
	    if(RoundRobinLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	    {
		for(unsigned int i = 0; i < _replicas.size(); ++i)
		{
		    replicas.push_back(_replicas[(_lastReplica + i) % _replicas.size()]);
		}
		_lastReplica = (_lastReplica + 1) % _replicas.size();
	    }
	    else if(AdaptiveLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	    {
		replicas = _replicas;
		adaptive = true;
		loadSample = _loadSample;
	    }
	    else// if(RandomLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	    {
		replicas = _replicas;
		random_shuffle(replicas.begin(), replicas.end());
	    }
	}
    }

    if(adaptive)
    {
	//
	// This must be done outside the synchronization block since
	// the sort() will call and lock each server entry.
	//
	random_shuffle(replicas.begin(), replicas.end());
	sort(replicas.begin(), replicas.end(), ServerLoadCI(loadSample));
    }

    //
    // Retrieve the proxy of each adapter from the server. The adapter
    // might not exist anymore at this time or the node might not be
    // reachable.
    //
    vector<pair<string, AdapterPrx> > adapters;
    auto_ptr<Ice::UserException> exception;
    for(ReplicaSeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
	try
	{
	    adapters.push_back(make_pair(p->second->getId(), p->second->getAdapter(_id, p->first)));
	}
	catch(AdapterNotExistException&)
	{
	}
	catch(const NodeUnreachableException& ex)
	{
	    exception.reset(dynamic_cast<NodeUnreachableException*>(ex.ice_clone()));
	}
    }

    if(adapters.empty())
    {
	if(!exception.get())
	{
	    throw AdapterNotExistException(_id, "");
	}
	else
	{
	    exception->ice_throw();
	}
    }

    return adapters;
}

string
AdapterEntry::getApplication() const
{
    Lock sync(*this);
    if(_replicas.empty())
    {
	throw AdapterNotExistException(_id, "");
    }
    return _replicas[0].second->getApplication();
}

AdapterPrx
AdapterEntry::getProxy(const string& replicaId) const
{
    pair<string, ServerEntryPtr> replica;
    bool replicated;
    {
	Lock sync(*this);
	if(_replicas.empty())
	{
	    throw AdapterNotExistException(_id, (_replicated ? replicaId : ""));
	}

	replicated = _replicated;
	if(!_replicated)
	{
	    replica = _replicas[0];
	}
	else
	{
	    for(ReplicaSeq::const_iterator p = _replicas.begin(); p != _replicas.end(); ++p)
	    {
		if(p->first == replicaId)
		{
		    replica = *p;
		    break;
		}
	    }
	}
    }

    if(replica.second)
    {
	try
	{
	    return replica.second->getAdapter(_id, replica.first);
	}
	catch(AdapterNotExistException&)
	{
	}
    }

    throw AdapterNotExistException(_id, (replicated ? replicaId : ""));
}

bool
AdapterEntry::canRemove()
{
    Lock sync(*this);
    return _replicas.empty() && !_replicated;
}
