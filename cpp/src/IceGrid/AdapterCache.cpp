// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Locator.h>
#include <IceGrid/AdapterCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

pointer_to_unary_function<int, int> ReplicaGroupEntry::_rand(IceUtil::random);

namespace IceGrid
{

struct ReplicaLoadComp : binary_function<ServerAdapterEntryPtr&, ServerAdapterEntryPtr&, bool>
{
    typedef ReplicaGroupEntry::ReplicaSeq::value_type Replica;
    typedef pair<float, Replica> ReplicaLoad;

    bool operator()(const ReplicaLoad& lhs, const ReplicaLoad& rhs)
    {
	return lhs.first < rhs.first;
    }
};

struct ToReplicaLoad : public unary_function<const ReplicaLoadComp::Replica&, ReplicaLoadComp::ReplicaLoad>
{
    ToReplicaLoad(LoadSample loadSample) : _loadSample(loadSample) { }

    ReplicaLoadComp::ReplicaLoad
    operator()(const ReplicaLoadComp::Replica& value)
    {
	try
	{
	    return make_pair(value.second->getLeastLoadedNodeLoad(_loadSample), value);
	}
	catch(const Ice::Exception&)
	{
	    return make_pair(1.0f, value);
	}
    }

    LoadSample _loadSample;
};

struct ToReplica : public unary_function<const ReplicaLoadComp::ReplicaLoad&, ReplicaLoadComp::Replica>
{
    ReplicaLoadComp::Replica
    operator()(const ReplicaLoadComp::ReplicaLoad& value)
    {
	return value.second;
    }
};

}

ServerAdapterEntryPtr
AdapterCache::addServerAdapter(const string& id, const string& rgId, bool allocatable, const ServerEntryPtr& server)
{
    Lock sync(*this);
    assert(!getImpl(id));
    ServerAdapterEntryPtr entry = new ServerAdapterEntry(*this, id, rgId, allocatable, server);
    addImpl(id, entry);

    if(!rgId.empty())
    {
	ReplicaGroupEntryPtr repEntry = ReplicaGroupEntryPtr::dynamicCast(getImpl(rgId));
	assert(repEntry);
	repEntry->addReplica(id, entry);
    }

    return entry;
}

ReplicaGroupEntryPtr
AdapterCache::addReplicaGroup(const string& id, const string& app, const LoadBalancingPolicyPtr& loadBalancing)
{
    Lock sync(*this);
    assert(!getImpl(id));
    ReplicaGroupEntryPtr entry = new ReplicaGroupEntry(*this, id, app, loadBalancing);
    addImpl(id, entry);
    return entry;
}

AdapterEntryPtr
AdapterCache::get(const string& id) const
{
    Lock sync(*this);
    AdapterEntryPtr entry = getImpl(id);
    if(!entry)
    {
	throw AdapterNotExistException(id);
    }
    return entry;
}

ServerAdapterEntryPtr
AdapterCache::getServerAdapter(const string& id) const
{
    Lock sync(*this);
    ServerAdapterEntryPtr svrEntry = ServerAdapterEntryPtr::dynamicCast(getImpl(id));
    if(!svrEntry)
    {
	throw AdapterNotExistException(id);
    }
    return svrEntry;
}

ReplicaGroupEntryPtr
AdapterCache::getReplicaGroup(const string& id) const
{
    Lock sync(*this);
    ReplicaGroupEntryPtr repEntry = ReplicaGroupEntryPtr::dynamicCast(getImpl(id));
    if(!repEntry)
    {
	throw AdapterNotExistException(id);
    }
    return repEntry;
}

void
AdapterCache::removeServerAdapter(const string& id)
{
    Lock sync(*this);

    ServerAdapterEntryPtr entry = ServerAdapterEntryPtr::dynamicCast(removeImpl(id));
    assert(entry);
    
    string replicaGroupId = entry->getReplicaGroupId();
    if(!replicaGroupId.empty())
    {
	ReplicaGroupEntryPtr repEntry = ReplicaGroupEntryPtr::dynamicCast(getImpl(replicaGroupId));
	assert(repEntry);
	repEntry->removeReplica(id);
    }
}

void
AdapterCache::removeReplicaGroup(const string& id)
{
    Lock sync(*this);
    ReplicaGroupEntryPtr entry = ReplicaGroupEntryPtr::dynamicCast(removeImpl(id));
    assert(entry);
}

AdapterEntryPtr
AdapterCache::addImpl(const string& id, const AdapterEntryPtr& entry)
{
    if(_traceLevels && _traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "added adapter `" << id << "'";	
    }    
    return Cache<string, AdapterEntry>::addImpl(id, entry);
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

AdapterEntry::AdapterEntry(AdapterCache& cache, const string& id, bool allocatable, const AllocatablePtr& parent) :
    Allocatable(allocatable, parent),
    _cache(cache),
    _id(id)
{
}

bool
AdapterEntry::canRemove()
{
    return true;
}

ServerAdapterEntry::ServerAdapterEntry(AdapterCache& cache,
				       const string& id,
				       const string& replicaGroupId, 
				       bool allocatable, 
				       const ServerEntryPtr& server) : 
    AdapterEntry(cache, id, allocatable, server),
    _replicaGroupId(replicaGroupId),
    _server(server)
{
}

vector<pair<string, AdapterPrx> >
ServerAdapterEntry::getProxies(int& nReplicas, const SessionIPtr& session)
{
    vector<pair<string, AdapterPrx> > adapters;
    try
    {
	nReplicas = 1;
	if(allocatable())
	{
	    if(session == getSession())
	    {
		adapters.push_back(make_pair(_id, getProxy()));
	    }
	}
	else
	{
	    adapters.push_back(make_pair(_id, getProxy()));
	}
    }
    catch(const NodeUnreachableException&)
    {
    }
    return adapters;
}

float
ServerAdapterEntry::getLeastLoadedNodeLoad(LoadSample loadSample) const
{
    return getServer()->getLoad(loadSample);
}

string
ServerAdapterEntry::getApplication() const
{
    return getServer()->getApplication();
}

AdapterInfoSeq
ServerAdapterEntry::getAdapterInfo() const
{
    AdapterInfo info;
    info.id = _id;
    info.replicaGroupId = _replicaGroupId;
    try
    {
	info.proxy = getProxy()->getDirectProxy();
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const AdapterNotExistException&)
    {
    }
    catch(const Ice::Exception&)
    {
    }
    AdapterInfoSeq infos;
    infos.push_back(info);
    return infos;
}

AdapterPrx
ServerAdapterEntry::getProxy(const string& replicaGroupId) const
{
    if(replicaGroupId.empty())
    {
	return getServer()->getAdapter(_id);
    }
    else
    {
	Lock sync(*this);
	if(_replicaGroupId != replicaGroupId)
	{
	    throw Ice::InvalidReplicaGroupIdException();
	}
	return _server->getAdapter(_id);
    }
}

ServerEntryPtr
ServerAdapterEntry::getServer() const
{
    Lock sync(*this);
    assert(_server);
    return _server;
}

void
ServerAdapterEntry::allocated(const SessionIPtr& session)
{
    TraceLevelsPtr traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->adapter > 1)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->adapterCat);
	out << "adapter `" << _id << "' allocated by `" << session->getUserId() << "' (" << _count << ")";
    }    
}

void
ServerAdapterEntry::released(const SessionIPtr& session)
{
    TraceLevelsPtr traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->adapter > 1)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->adapterCat);
	out << "adapter `" << _id << "' released by `" << session->getUserId() << "' (" << _count << ")";
    }    
}

ReplicaGroupEntry::ReplicaGroupEntry(AdapterCache& cache,
				     const string& id,
				     const string& application,
				     const LoadBalancingPolicyPtr& policy) : 
    AdapterEntry(cache, id, false, 0),
    _application(application),
    _lastReplica(0)
{
    update(policy);
}

void
ReplicaGroupEntry::addReplica(const string& replicaId, const ServerAdapterEntryPtr& adapter)
{
    Lock sync(*this);
    _replicas.push_back(make_pair(replicaId, adapter));
}

void
ReplicaGroupEntry::removeReplica(const string& replicaId)
{
    Lock sync(*this);
    for(ReplicaGroupEntry::ReplicaSeq::iterator p = _replicas.begin(); p != _replicas.end(); ++p)
    {
	if(replicaId == p->first)
	{
	    _replicas.erase(p);		
	    // Make sure _lastReplica is still within the bounds.
	    _lastReplica = _replicas.empty() ? 0 : _lastReplica % static_cast<int>(_replicas.size());
	    break;
	}
    }
}

void
ReplicaGroupEntry::update(const LoadBalancingPolicyPtr& policy)
{
    Lock sync(*this);
    _loadBalancing = policy;
    if(_loadBalancing)
    {
	istringstream is(_loadBalancing->nReplicas);
	int nReplicas = 0;
	is >> nReplicas;
	_loadBalancingNReplicas = nReplicas < 1 ? 1 : nReplicas;
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

vector<pair<string, AdapterPrx> >
ReplicaGroupEntry::getProxies(int& nReplicas, const SessionIPtr& session)
{
    ReplicaSeq replicas;
    bool adaptive = false;
    LoadSample loadSample = LoadSample1;
    {
	Lock sync(*this);
	if(_replicas.empty())
	{
	    return vector<pair<string, AdapterPrx> >();
	}

	nReplicas = _loadBalancingNReplicas > 0 ? _loadBalancingNReplicas : static_cast<int>(_replicas.size());
	replicas.reserve(_replicas.size());
	if(!_loadBalancing)
	{
	    replicas = _replicas;
	}
	if(RoundRobinLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	{
	    for(unsigned int i = 0; i < _replicas.size(); ++i)
	    {
		replicas.push_back(_replicas[(_lastReplica + i) % _replicas.size()]);
	    }
	    _lastReplica = (_lastReplica + 1) % static_cast<int>(_replicas.size());
	}
	else if(AdaptiveLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	{
	    replicas = _replicas;
	    random_shuffle(replicas.begin(), replicas.end(), _rand);
	    adaptive = true;
	    loadSample = _loadSample;
	}
	else// if(RandomLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
	{
	    replicas = _replicas;
	    random_shuffle(replicas.begin(), replicas.end(), _rand);
	}
    }

    if(adaptive)
    {
	//
	// This must be done outside the synchronization block since
	// the sort() will call and lock each server entry.
	//

	vector<ReplicaLoadComp::ReplicaLoad> rl;
	transform(replicas.begin(), replicas.end(), back_inserter(rl), ToReplicaLoad(loadSample));
	sort(rl.begin(), rl.end(), ReplicaLoadComp());
	replicas.clear();
	transform(rl.begin(), rl.end(), back_inserter(replicas), ToReplica());
    }

    //
    // Retrieve the proxy of each adapter from the server. The adapter
    // might not exist anymore at this time or the node might not be
    // reachable.
    //
    vector<pair<string, AdapterPrx> > adapters;
    for(ReplicaSeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
	try
	{
	    if(p->second->allocatable())
	    {
		if(session == p->second->getSession())
		{
		    adapters.push_back(make_pair(p->first, p->second->getProxy()));
		}
	    }
	    else
	    {
		adapters.push_back(make_pair(p->first, p->second->getProxy()));
	    }
	}
	catch(AdapterNotExistException&)
	{
	}
	catch(const NodeUnreachableException&)
	{
	}
    }
    return adapters;
}

float
ReplicaGroupEntry::getLeastLoadedNodeLoad(LoadSample loadSample) const
{
    ReplicaSeq replicas;
    {
	Lock sync(*this);
	replicas = _replicas;
    }

    //
    // This must be done outside the synchronization block since
    // min_element() will call and lock each server entry.
    //
    random_shuffle(replicas.begin(), replicas.end(), _rand);
    vector<ReplicaLoadComp::ReplicaLoad> rl;
    transform(replicas.begin(), replicas.end(), back_inserter(rl), ToReplicaLoad(loadSample));
    AdapterEntryPtr adpt = min_element(rl.begin(), rl.end(), ReplicaLoadComp())->second.second;
    return adpt->getLeastLoadedNodeLoad(loadSample);
}

string
ReplicaGroupEntry::getApplication() const
{
    Lock sync(*this);
    return _application;
}

AdapterInfoSeq
ReplicaGroupEntry::getAdapterInfo() const
{
    ReplicaSeq replicas;
    {
	Lock sync(*this);
	replicas = _replicas;
    }

    AdapterInfoSeq infos;
    for(ReplicaSeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
	AdapterInfoSeq infs = p->second->getAdapterInfo();
	assert(infs.size() == 1);
	infos.push_back(infs[0]);
    }
    return infos;
}
