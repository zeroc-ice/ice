//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Random.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Locator.h>
#include <IceGrid/AdapterCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/SessionI.h>

#include <functional>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

struct ReplicaLoadComp : binary_function<ServerAdapterEntryPtr&, ServerAdapterEntryPtr&, bool>
{
    bool operator()(const pair<float, ServerAdapterEntryPtr>& lhs, const pair<float, ServerAdapterEntryPtr>& rhs)
    {
        return lhs.first < rhs.first;
    }
};

struct ReplicaPriorityComp : binary_function<ServerAdapterEntryPtr&, ServerAdapterEntryPtr&, bool>
{
    bool operator()(const ServerAdapterEntryPtr& lhs, const ServerAdapterEntryPtr& rhs)
    {
        return lhs->getPriority() < rhs->getPriority();
    }
};

struct TransformToReplicaLoad :
        public unary_function<const ServerAdapterEntryPtr&, pair<float, ServerAdapterEntryPtr> >
{
public:

    TransformToReplicaLoad(LoadSample loadSample) : _loadSample(loadSample) { }

    pair<float, ServerAdapterEntryPtr>
    operator()(const ServerAdapterEntryPtr& value)
    {
        return make_pair(value->getLeastLoadedNodeLoad(_loadSample), value);
    }

    LoadSample _loadSample;
};

struct TransformToReplica : public unary_function<const pair<string, ServerAdapterEntryPtr>&, ServerAdapterEntryPtr>
{
    ServerAdapterEntryPtr
    operator()(const pair<float, ServerAdapterEntryPtr>& value)
    {
        return value.second;
    }
};

class ReplicaGroupSyncCallback : public SynchronizationCallback, public IceUtil::Mutex
{
public:

    ReplicaGroupSyncCallback(const SynchronizationCallbackPtr& callback, int count, int nReplicas) :
        _callback(callback),
        _responseCalled(false),
        _synchronizeCount(count),
        _synchronizedCount(0),
        _nReplicas(nReplicas > count ? count : nReplicas)
    {
    }

    bool
    response()
    {
        Lock sync(*this);
        _responseCalled = true;
        if(_synchronizedCount >= _nReplicas)
        {
            _callback = 0;
            return false;
        }
        else if(_synchronizeCount == 0)
        {
            if(_synchronizedCount == 0 && _exception.get())
            {
                _exception->ice_throw();
            }
            _callback = 0;
            return false;
        }
        return true;
    }

    void
    synchronized()
    {
        SynchronizationCallbackPtr callback;
        {
            Lock sync(*this);
            ++_synchronizedCount;
            --_synchronizeCount;

            if(!_responseCalled)
            {
                return;
            }

            if(_synchronizedCount < _nReplicas && _synchronizeCount > 0)
            {
                return;
            }

            callback = _callback;
            _callback = 0;
        }

        if(callback)
        {
            callback->synchronized();
        }
    }

    void
    synchronized(const Ice::Exception& ex)
    {
        SynchronizationCallbackPtr callback;
        {
            Lock sync(*this);
            if(!_exception.get())
            {
                _exception.reset(ex.ice_clone());
            }

            --_synchronizeCount;
            if(!_responseCalled)
            {
                return;
            }

            if(_synchronizeCount > 0)
            {
                return;
            }

            callback = _callback;
            _callback = 0;
        }

        if(callback)
        {
            callback->synchronized(ex);
        }
    }

private:

    SynchronizationCallbackPtr _callback;
    bool _responseCalled;
    int _synchronizeCount;
    int _synchronizedCount;
    int _nReplicas;
    IceInternal::UniquePtr<Ice::Exception> _exception;
};
typedef IceUtil::Handle<ReplicaGroupSyncCallback> ReplicaGroupSyncCallbackPtr;

}

AdapterCache::AdapterCache(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
AdapterCache::addServerAdapter(const AdapterDescriptor& desc, const ServerEntryPtr& server, const string& app)
{
    Lock sync(*this);
    if(getImpl(desc.id))
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't add duplicate adapter `" << desc.id << "'";
        return;
    }

    istringstream is(desc.priority);
    int priority = 0;
    is >> priority;

    ServerAdapterEntryPtr entry = new ServerAdapterEntry(*this, desc.id, app, desc.replicaGroupId, priority, server);
    addImpl(desc.id, entry);

    if(!desc.replicaGroupId.empty())
    {
        ReplicaGroupEntryPtr repEntry = ReplicaGroupEntryPtr::dynamicCast(getImpl(desc.replicaGroupId));
        if(!repEntry)
        {
            //
            // Add an un-assigned replica group, the replica group will in theory be added
            // shortly after when its application is loaded.
            //
            repEntry = new ReplicaGroupEntry(*this, desc.replicaGroupId, "", new RandomLoadBalancingPolicy("0"), "");
            addImpl(desc.replicaGroupId, repEntry);
        }
        repEntry->addReplica(desc.id, entry);
    }
}

void
AdapterCache::addReplicaGroup(const ReplicaGroupDescriptor& desc, const string& app)
{
    Lock sync(*this);
    ReplicaGroupEntryPtr repEntry = ReplicaGroupEntryPtr::dynamicCast(getImpl(desc.id));
    if(repEntry)
    {
        //
        // If the replica group isn't assigned to an application,
        // assign it. Otherwise, it's a duplicate so we log an error.
        //
        if(repEntry->getApplication().empty())
        {
            repEntry->update(app, desc.loadBalancing, desc.filter);
        }
        else
        {
            Ice::Error out(_communicator->getLogger());
            out << "can't add duplicate replica group `" << desc.id << "'";
        }
        return;
    }
    addImpl(desc.id, new ReplicaGroupEntry(*this, desc.id, app, desc.loadBalancing, desc.filter));
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

void
AdapterCache::removeServerAdapter(const string& id)
{
    Lock sync(*this);

    ServerAdapterEntryPtr entry = ServerAdapterEntryPtr::dynamicCast(getImpl(id));
    if(!entry)
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't remove unknown adapter `" << id << "'";
        return;
    }
    removeImpl(id);

    string replicaGroupId = entry->getReplicaGroupId();
    if(!replicaGroupId.empty())
    {
        ReplicaGroupEntryPtr repEntry = ReplicaGroupEntryPtr::dynamicCast(getImpl(replicaGroupId));
        if(!repEntry)
        {
            Ice::Error out(_communicator->getLogger());
            out << "can't remove adapter `" << id << "' from unknown replica group `" << replicaGroupId << "'";
        }
        else
        {
            //
            // If the replica group is empty and it's not assigned, remove it.
            //
            if(repEntry->removeReplica(id))
            {
                removeImpl(replicaGroupId);
            }
        }
    }
}

void
AdapterCache::removeReplicaGroup(const string& id)
{
    Lock sync(*this);
    ReplicaGroupEntryPtr entry = ReplicaGroupEntryPtr::dynamicCast(getImpl(id));
    if(!entry)
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't remove unknown replica group `" << id << "'";
        return;
    }
    removeImpl(id);
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

void
AdapterCache::removeImpl(const string& id)
{
    if(_traceLevels && _traceLevels->adapter > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
        out << "removed adapter `" << id << "'";
    }
    Cache<string, AdapterEntry>::removeImpl(id);
}

AdapterEntry::AdapterEntry(AdapterCache& cache, const string& id, const string& application) :
    _cache(cache),
    _id(id),
    _application(application)
{
}

bool
AdapterEntry::canRemove()
{
    return true;
}

string
AdapterEntry::getId() const
{
    return _id;
}

string
AdapterEntry::getApplication() const
{
    return _application;
}

ServerAdapterEntry::ServerAdapterEntry(AdapterCache& cache,
                                       const string& id,
                                       const string& application,
                                       const string& replicaGroupId,
                                       int priority,
                                       const ServerEntryPtr& server) :
    AdapterEntry(cache, id, application),
    _replicaGroupId(replicaGroupId),
    _priority(priority),
    _server(server)
{
}

bool
ServerAdapterEntry::addSyncCallback(const SynchronizationCallbackPtr& callback, const set<string>&)
{
    try
    {
        return _server->addSyncCallback(callback);
    }
    catch(const ServerNotExistException&)
    {
        throw AdapterNotExistException(_id);
    }
}

void
ServerAdapterEntry::getLocatorAdapterInfo(LocatorAdapterInfoSeq& adapters, int& nReplicas, bool& replicaGroup,
                                          bool& roundRobin, string&, const set<string>&)
{
    nReplicas = 1;
    replicaGroup = false;
    roundRobin = false;
    getLocatorAdapterInfo(adapters);
}

float
ServerAdapterEntry::getLeastLoadedNodeLoad(LoadSample loadSample) const
{
    try
    {
        return _server->getLoad(loadSample);
    }
    catch(const ServerNotExistException&)
    {
        // This might happen if the application is updated concurrently.
    }
    catch(const NodeNotExistException&)
    {
        // This might happen if the application is updated concurrently.
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const Ice::Exception& ex)
    {
        Ice::Error error(_cache.getTraceLevels()->logger);
        error << "unexpected exception while getting node load:\n" << ex;
    }
    return 999.9f;
}

AdapterInfoSeq
ServerAdapterEntry::getAdapterInfo() const
{
    AdapterInfo info;
    info.id = _id;
    info.replicaGroupId = _replicaGroupId;
    try
    {
        info.proxy = _server->getAdapter(_id, true)->getDirectProxy();
    }
    catch(const SynchronizationException&)
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
ServerAdapterEntry::getProxy(const string& replicaGroupId, bool upToDate) const
{
    if(replicaGroupId.empty())
    {
        return _server->getAdapter(_id, upToDate);
    }
    else
    {
        if(_replicaGroupId != replicaGroupId) // Validate the replica group.
        {
            throw Ice::InvalidReplicaGroupIdException();
        }
        return _server->getAdapter(_id, upToDate);
    }
}

void
ServerAdapterEntry::getLocatorAdapterInfo(LocatorAdapterInfoSeq& adapters) const
{
    LocatorAdapterInfo info;
    info.id = _id;
    info.proxy = _server->getAdapter(info.activationTimeout, info.deactivationTimeout, _id, true);
    adapters.push_back(info);
}

int
ServerAdapterEntry::getPriority() const
{
    return _priority;
}

string
ServerAdapterEntry::getServerId() const
{
    return _server->getId();
}

string
ServerAdapterEntry::getNodeName() const
{
    try
    {
        return _server->getInfo().node;
    }
    catch(const ServerNotExistException&)
    {
        return "";
    }
}

ReplicaGroupEntry::ReplicaGroupEntry(AdapterCache& cache,
                                     const string& id,
                                     const string& application,
                                     const LoadBalancingPolicyPtr& policy,
                                     const string& filter) :
    AdapterEntry(cache, id, application),
    _lastReplica(0),
    _requestInProgress(false)
{
    update(application, policy, filter);
}

bool
ReplicaGroupEntry::addSyncCallback(const SynchronizationCallbackPtr& callback, const set<string>& excludes)
{
    vector<ServerAdapterEntryPtr> replicas;
    int nReplicas;
    int roundRobin = false;
    {
        Lock sync(*this);
        nReplicas = _loadBalancingNReplicas > 0 ? _loadBalancingNReplicas : static_cast<int>(_replicas.size());
        roundRobin = RoundRobinLoadBalancingPolicyPtr::dynamicCast(_loadBalancing);
        if(!roundRobin)
        {
            replicas = _replicas;
        }
        else
        {
            for(vector<ServerAdapterEntryPtr>::const_iterator p = _replicas.begin(); p != _replicas.end(); ++p)
            {
                if(excludes.find((*p)->getId()) == excludes.end())
                {
                    replicas.push_back(*p);
                }
            }
        }

        if(replicas.empty())
        {
            return false;
        }
    }

    ReplicaGroupSyncCallbackPtr cb = new ReplicaGroupSyncCallback(callback,
                                                                  static_cast<int>(replicas.size()),
                                                                  nReplicas);
    set<string> emptyExcludes;
    for(vector<ServerAdapterEntryPtr>::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
        try
        {
            if(!(*p)->addSyncCallback(cb, emptyExcludes))
            {
                cb->synchronized();
            }
        }
        catch(const Ice::Exception& ex)
        {
            cb->synchronized(ex);
        }
    }
    return cb->response();
}

void
ReplicaGroupEntry::addReplica(const string& /*replicaId*/, const ServerAdapterEntryPtr& adapter)
{
    Lock sync(*this);
    _replicas.push_back(adapter);
}

bool
ReplicaGroupEntry::removeReplica(const string& replicaId)
{
    Lock sync(*this);
    for(vector<ServerAdapterEntryPtr>::iterator p = _replicas.begin(); p != _replicas.end(); ++p)
    {
        if(replicaId == (*p)->getId())
        {
            _replicas.erase(p);
            // Make sure _lastReplica is still within the bounds.
            _lastReplica = _replicas.empty() ? 0 : _lastReplica % static_cast<int>(_replicas.size());
            break;
        }
    }

    // Replica group can be removed if not assigned to an application and there's no more replicas
    return _replicas.empty() && _application.empty();
}

void
ReplicaGroupEntry::update(const string& application, const LoadBalancingPolicyPtr& policy, const string& filter)
{
    Lock sync(*this);
    assert(policy);

    _application = application;
    _loadBalancing = policy;
    _filter = filter;

    istringstream is(_loadBalancing->nReplicas);
    int nReplicas = 0;
    is >> nReplicas;
    _loadBalancingNReplicas = nReplicas < 0 ? 1 : nReplicas;
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

void
ReplicaGroupEntry::getLocatorAdapterInfo(LocatorAdapterInfoSeq& adapters, int& nReplicas, bool& replicaGroup,
                                         bool& roundRobin, string& filter, const set<string>& excludes)
{
    vector<ServerAdapterEntryPtr> replicas;
    bool adaptive = false;
    LoadSample loadSample = LoadSample1;
    {
        Lock sync(*this);
        replicaGroup = true;
        roundRobin = false;
        filter = _filter;
        nReplicas = _loadBalancingNReplicas > 0 ? _loadBalancingNReplicas : static_cast<int>(_replicas.size());

        if(_replicas.empty())
        {
            return;
        }

        replicas.reserve(_replicas.size());
        if(RoundRobinLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
        {
            // Serialize round-robin requests
            while(_requestInProgress)
            {
                wait();
            }
            _requestInProgress = true;
            for(size_t i = 0; i < _replicas.size(); ++i)
            {
                replicas.push_back(_replicas[(static_cast<size_t>(_lastReplica) + i) % _replicas.size()]);
            }
            _lastReplica = (_lastReplica + 1) % static_cast<int>(_replicas.size());
            roundRobin = true;
        }
        else if(AdaptiveLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
        {
            replicas = _replicas;
            IceUtilInternal::shuffle(replicas.begin(), replicas.end());
            loadSample = _loadSample;
            adaptive = true;
        }
        else if(OrderedLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
        {
            replicas = _replicas;
            sort(replicas.begin(), replicas.end(), ReplicaPriorityComp());
        }
        else if(RandomLoadBalancingPolicyPtr::dynamicCast(_loadBalancing))
        {
            replicas = _replicas;
            IceUtilInternal::shuffle(replicas.begin(), replicas.end());
        }
    }

    int unreachable = 0;
    bool synchronizing = false;
    try
    {
        if(adaptive)
        {
            //
            // This must be done outside the synchronization block since
            // the trasnform() might call and lock each server adapter
            // entry. We also can't sort directly as the load of each
            // server adapter is not stable so we first take a snapshot of
            // each adapter and sort the snapshot.
            //
            vector<pair<float, ServerAdapterEntryPtr> > rl;
            transform(replicas.begin(), replicas.end(), back_inserter(rl), TransformToReplicaLoad(loadSample));
            sort(rl.begin(), rl.end(), ReplicaLoadComp());
            replicas.clear();
            transform(rl.begin(), rl.end(), back_inserter(replicas), TransformToReplica());
        }

        //
        // Retrieve the proxy of each adapter from the server. The adapter
        // might not exist anymore at this time or the node might not be
        // reachable.
        //
        set<string> emptyExcludes;
        bool firstUnreachable = true;
        for(vector<ServerAdapterEntryPtr>::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
        {
            if(!roundRobin || excludes.find((*p)->getId()) == excludes.end())
            {
                try
                {
                    (*p)->getLocatorAdapterInfo(adapters);
                    firstUnreachable = false;
                }
                catch(const SynchronizationException&)
                {
                    synchronizing = true;
                }
                catch(const Ice::UserException&)
                {
                    if(firstUnreachable)
                    {
                        ++unreachable; // Count the number of un-reachable nodes.
                    }
                }
            }
        }
    }
    catch(...)
    {
        if(roundRobin)
        {
            Lock sync(*this);
            assert(_requestInProgress);
            _requestInProgress = false;
            notify();
        }
        throw;
    }

    if(roundRobin)
    {
        Lock sync(*this);
        assert(_requestInProgress);
        _requestInProgress = false;
        notify();
        if(unreachable > 0)
        {
            _lastReplica = (_lastReplica + unreachable) % static_cast<int>(_replicas.size());
        }
    }

    if(adapters.empty() && synchronizing)
    {
        throw SynchronizationException(__FILE__, __LINE__);
    }
}

float
ReplicaGroupEntry::getLeastLoadedNodeLoad(LoadSample loadSample) const
{
    vector<ServerAdapterEntryPtr> replicas;
    {
        Lock sync(*this);
        replicas = _replicas;
    }

    if(replicas.empty())
    {
        return 999.9f;
    }
    else if(replicas.size() == 1)
    {
        return replicas.back()->getLeastLoadedNodeLoad(loadSample);
    }
    else
    {
        IceUtilInternal::shuffle(replicas.begin(), replicas.end());
        vector<pair<float, ServerAdapterEntryPtr> > rl;
        transform(replicas.begin(), replicas.end(), back_inserter(rl), TransformToReplicaLoad(loadSample));
        return min_element(rl.begin(), rl.end(), ReplicaLoadComp())->first;
    }
}

AdapterInfoSeq
ReplicaGroupEntry::getAdapterInfo() const
{
    //
    // This method is called with the database locked so we're sure
    // that no new adapters will be added or removed concurrently.
    //
    vector<ServerAdapterEntryPtr> replicas;
    {
        Lock sync(*this);
        replicas = _replicas;
    }

    AdapterInfoSeq infos;
    for(vector<ServerAdapterEntryPtr>::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
        AdapterInfoSeq infs = (*p)->getAdapterInfo();
        assert(infs.size() == 1);
        infos.push_back(infs[0]);
    }
    return infos;
}

bool
ReplicaGroupEntry::hasAdaptersFromOtherApplications() const
{
    vector<ServerAdapterEntryPtr> replicas;
    {
        Lock sync(*this);
        replicas = _replicas;
    }

    AdapterInfoSeq infos;
    for(vector<ServerAdapterEntryPtr>::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
        if((*p)->getApplication() != _application)
        {
            return true;
        }
    }
    return false;
}
