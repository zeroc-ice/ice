// Copyright (c) ZeroC, Inc.

#include "AdapterCache.h"
#include "../Ice/Random.h"
#include "Ice/Communicator.h"
#include "Ice/Locator.h"
#include "Ice/LoggerUtil.h"
#include "NodeCache.h"
#include "NodeSessionI.h"
#include "ServerCache.h"
#include "SessionI.h"

#include <functional>

#include "SynchronizationException.h"

using namespace std;
using namespace IceGrid;

namespace IceGrid
{
    class ReplicaGroupSyncCallback final : public SynchronizationCallback
    {
    public:
        ReplicaGroupSyncCallback(const shared_ptr<SynchronizationCallback>& callback, int count, int nReplicas)
            : _callback(callback),
              _synchronizeCount(count),
              _nReplicas(nReplicas > count ? count : nReplicas)
        {
        }

        bool response()
        {
            lock_guard lock(_mutex);
            _responseCalled = true;
            if (_synchronizedCount >= _nReplicas)
            {
                _callback = nullptr;
                return false;
            }
            else if (_synchronizeCount == 0)
            {
                if (_synchronizedCount == 0 && _exception)
                {
                    rethrow_exception(_exception);
                }
                _callback = nullptr;
                return false;
            }
            return true;
        }

        void synchronized() override
        {
            shared_ptr<SynchronizationCallback> callback;
            {
                lock_guard lock(_mutex);
                ++_synchronizedCount;
                --_synchronizeCount;

                if (!_responseCalled)
                {
                    return;
                }

                if (_synchronizedCount < _nReplicas && _synchronizeCount > 0)
                {
                    return;
                }

                callback = _callback;
                _callback = nullptr;
            }

            if (callback)
            {
                callback->synchronized();
            }
        }

        void synchronized(exception_ptr ex) override
        {
            shared_ptr<SynchronizationCallback> callback;
            {
                lock_guard lock(_mutex);
                if (!_exception)
                {
                    _exception = ex;
                }

                --_synchronizeCount;
                if (!_responseCalled)
                {
                    return;
                }

                if (_synchronizeCount > 0)
                {
                    return;
                }

                callback = _callback;
                _callback = nullptr;
            }

            if (callback)
            {
                callback->synchronized(ex);
            }
        }

    private:
        shared_ptr<SynchronizationCallback> _callback;
        bool _responseCalled{false};
        int _synchronizeCount;
        int _synchronizedCount{0};
        int _nReplicas;
        exception_ptr _exception;

        mutex _mutex;
    };
}

void
GetAdapterInfoResult::add(const ServerAdapterEntry* adapter)
{
    AdapterInfo info;
    info.id = adapter->getId();
    info.replicaGroupId = adapter->getReplicaGroupId();
    _adapters.push_back(info);
    try
    {
        _results.emplace_back(adapter->getProxy("", true)->getDirectProxyAsync());
    }
    catch (const Ice::Exception&)
    {
        _results.emplace_back(nullopt);
    }
}

AdapterInfoSeq
GetAdapterInfoResult::get()
{
    auto q = _adapters.begin();
    for (auto p = _results.begin(); p != _results.end(); ++p, ++q)
    {
        try
        {
            if (*p)
            {
                q->proxy = Ice::uncheckedCast<AdapterPrx>((*p)->get());
            }
        }
        catch (const Ice::Exception&)
        {
        }
    }
    return _adapters;
}

AdapterCache::AdapterCache(const shared_ptr<Ice::Communicator>& communicator) : _communicator(communicator) {}

void
AdapterCache::addServerAdapter(const AdapterDescriptor& desc, const shared_ptr<ServerEntry>& server, const string& app)
{
    lock_guard lock(_mutex);
    if (getImpl(desc.id))
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't add duplicate adapter '" << desc.id << "'";
        return;
    }

    int priority = 0;
    try
    {
        priority = stoi(desc.priority);
    }
    catch (const std::exception&)
    {
    }

    auto entry = make_shared<ServerAdapterEntry>(*this, desc.id, app, desc.replicaGroupId, priority, server);
    addImpl(desc.id, entry);

    if (!desc.replicaGroupId.empty())
    {
        auto repEntry = dynamic_pointer_cast<ReplicaGroupEntry>(getImpl(desc.replicaGroupId));
        if (!repEntry)
        {
            //
            // Add an un-assigned replica group, the replica group will in theory be added
            // shortly after when its application is loaded.
            //
            repEntry = make_shared<ReplicaGroupEntry>(
                *this,
                desc.replicaGroupId,
                "",
                make_shared<RandomLoadBalancingPolicy>("0"),
                "");
            addImpl(desc.replicaGroupId, repEntry);
        }
        repEntry->addReplica(desc.id, entry);
    }
}

void
AdapterCache::addReplicaGroup(const ReplicaGroupDescriptor& desc, const string& app)
{
    lock_guard lock(_mutex);
    auto repEntry = dynamic_pointer_cast<ReplicaGroupEntry>(getImpl(desc.id));
    if (repEntry)
    {
        //
        // If the replica group isn't assigned to an application,
        // assign it. Otherwise, it's a duplicate so we log an error.
        //
        if (repEntry->getApplication().empty())
        {
            repEntry->update(app, desc.loadBalancing, desc.filter);
        }
        else
        {
            Ice::Error out(_communicator->getLogger());
            out << "can't add duplicate replica group '" << desc.id << "'";
        }
        return;
    }
    addImpl(desc.id, make_shared<ReplicaGroupEntry>(*this, desc.id, app, desc.loadBalancing, desc.filter));
}

shared_ptr<AdapterEntry>
AdapterCache::get(const string& id) const
{
    lock_guard lock(_mutex);

    auto entry = getImpl(id);
    if (!entry)
    {
        throw AdapterNotExistException(id);
    }
    return entry;
}

void
AdapterCache::removeServerAdapter(const string& id)
{
    lock_guard lock(_mutex);

    auto entry = dynamic_pointer_cast<ServerAdapterEntry>(getImpl(id));
    if (!entry)
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't remove unknown adapter '" << id << "'";
        return;
    }
    removeImpl(id);

    string replicaGroupId = entry->getReplicaGroupId();
    if (!replicaGroupId.empty())
    {
        auto repEntry = dynamic_pointer_cast<ReplicaGroupEntry>(getImpl(replicaGroupId));
        if (!repEntry)
        {
            Ice::Error out(_communicator->getLogger());
            out << "can't remove adapter '" << id << "' from unknown replica group '" << replicaGroupId << "'";
        }
        else
        {
            //
            // If the replica group is empty and it's not assigned, remove it.
            //
            if (repEntry->removeReplica(id))
            {
                removeImpl(replicaGroupId);
            }
        }
    }
}

void
AdapterCache::removeReplicaGroup(const string& id)
{
    lock_guard lock(_mutex);

    auto entry = dynamic_pointer_cast<ReplicaGroupEntry>(getImpl(id));
    if (!entry)
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't remove unknown replica group '" << id << "'";
        return;
    }
    removeImpl(id);
}

shared_ptr<AdapterEntry>
AdapterCache::addImpl(const string& id, const shared_ptr<AdapterEntry>& entry)
{
    if (_traceLevels && _traceLevels->adapter > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
        out << "added adapter '" << id << "'";
    }
    return Cache<string, AdapterEntry>::addImpl(id, entry);
}

void
AdapterCache::removeImpl(const string& id)
{
    if (_traceLevels && _traceLevels->adapter > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
        out << "removed adapter '" << id << "'";
    }
    Cache<string, AdapterEntry>::removeImpl(id);
}

AdapterEntry::AdapterEntry(AdapterCache& cache, string id, string application)
    : _cache(cache),
      _id(std::move(id)),
      _application(std::move(application))
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

ServerAdapterEntry::ServerAdapterEntry(
    AdapterCache& cache,
    const string& id,
    const string& application,
    string replicaGroupId,
    int priority,
    const shared_ptr<ServerEntry>& server)
    : AdapterEntry(cache, id, application),
      _replicaGroupId(std::move(replicaGroupId)),
      _priority(priority),
      _server(server)
{
}

bool
ServerAdapterEntry::addSyncCallback(const shared_ptr<SynchronizationCallback>& callback, const set<string>&)
{
    try
    {
        return _server->addSyncCallback(callback);
    }
    catch (const ServerNotExistException&)
    {
        throw AdapterNotExistException(_id);
    }
}

void
ServerAdapterEntry::getLocatorAdapterInfo(
    LocatorAdapterInfoSeq& adapters,
    int& nReplicas,
    bool& replicaGroup,
    bool& roundRobin,
    string&,
    const set<string>&)
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
    catch (const ServerNotExistException&)
    {
        // This might happen if the application is updated concurrently.
    }
    catch (const NodeNotExistException&)
    {
        // This might happen if the application is updated concurrently.
    }
    catch (const NodeUnreachableException&)
    {
    }
    catch (const Ice::Exception& ex)
    {
        Ice::Error error(_cache.getTraceLevels()->logger);
        error << "unexpected exception while getting node load:\n" << ex;
    }
    return 999.9f;
}

AdapterInfoSeq
ServerAdapterEntry::getAdapterInfoNoEndpoints() const
{
    return {{_id, nullopt, _replicaGroupId}};
}

shared_ptr<GetAdapterInfoResult>
ServerAdapterEntry::getAdapterInfoAsync() const
{
    auto result = make_shared<GetAdapterInfoResult>();
    result->add(this);
    return result;
}

optional<AdapterPrx>
ServerAdapterEntry::getProxy(const string& replicaGroupId, bool upToDate) const
{
    if (replicaGroupId.empty())
    {
        return _server->getAdapter(_id, upToDate);
    }
    else
    {
        if (_replicaGroupId != replicaGroupId) // Validate the replica group.
        {
            throw Ice::InvalidReplicaGroupIdException();
        }
        return _server->getAdapter(_id, upToDate);
    }
}

void
ServerAdapterEntry::getLocatorAdapterInfo(LocatorAdapterInfoSeq& adapters) const
{
    chrono::seconds activationTimeout, deactivationTimeout;
    auto proxy = _server->getAdapter(activationTimeout, deactivationTimeout, _id, true);

    LocatorAdapterInfo info = {_id, std::move(proxy), activationTimeout, deactivationTimeout};
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
    catch (const ServerNotExistException&)
    {
        return "";
    }
}

ReplicaGroupEntry::ReplicaGroupEntry(
    AdapterCache& cache,
    const string& id,
    const string& application,
    const shared_ptr<LoadBalancingPolicy>& policy,
    const string& filter)
    : AdapterEntry(cache, id, application)
{
    update(application, policy, filter);
}

bool
ReplicaGroupEntry::addSyncCallback(const shared_ptr<SynchronizationCallback>& callback, const set<string>& excludes)
{
    vector<shared_ptr<ServerAdapterEntry>> replicas;
    int nReplicas;
    bool roundRobin = false;
    {
        lock_guard lock(_mutex);

        nReplicas = _loadBalancingNReplicas > 0 ? _loadBalancingNReplicas : static_cast<int>(_replicas.size());
        roundRobin = dynamic_pointer_cast<RoundRobinLoadBalancingPolicy>(_loadBalancing) != nullptr;
        if (!roundRobin)
        {
            replicas = _replicas;
        }
        else
        {
            for (const auto& replica : _replicas)
            {
                if (excludes.find(replica->getId()) == excludes.end())
                {
                    replicas.push_back(replica);
                }
            }
        }

        if (replicas.empty())
        {
            return false;
        }
    }

    auto cb = make_shared<ReplicaGroupSyncCallback>(callback, static_cast<int>(replicas.size()), nReplicas);
    set<string> emptyExcludes;
    for (const auto& replica : replicas)
    {
        try
        {
            if (!replica->addSyncCallback(cb, emptyExcludes))
            {
                cb->synchronized();
            }
        }
        catch (const std::exception&)
        {
            cb->synchronized(current_exception());
        }
    }
    return cb->response();
}

void
ReplicaGroupEntry::addReplica(const string& /*replicaId*/, const shared_ptr<ServerAdapterEntry>& adapter)
{
    lock_guard lock(_mutex);
    _replicas.push_back(adapter);
}

bool
ReplicaGroupEntry::removeReplica(const string& replicaId)
{
    lock_guard lock(_mutex);
    for (auto p = _replicas.cbegin(); p != _replicas.cend(); ++p)
    {
        if (replicaId == (*p)->getId())
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
ReplicaGroupEntry::update(
    const string& application,
    const shared_ptr<LoadBalancingPolicy>& policy,
    const string& filter)
{
    lock_guard lock(_mutex);

    assert(policy);

    _application = application;
    _loadBalancing = policy;
    _filter = filter;

    int nReplicas = 0;
    try
    {
        nReplicas = stoi(_loadBalancing->nReplicas);
    }
    catch (const std::exception&)
    {
    }

    _loadBalancingNReplicas = nReplicas < 0 ? 1 : nReplicas;
    auto alb = dynamic_pointer_cast<AdaptiveLoadBalancingPolicy>(_loadBalancing);
    if (alb)
    {
        if (alb->loadSample == "1")
        {
            _loadSample = LoadSample::LoadSample1;
        }
        else if (alb->loadSample == "5")
        {
            _loadSample = LoadSample::LoadSample5;
        }
        else if (alb->loadSample == "15")
        {
            _loadSample = LoadSample::LoadSample15;
        }
        else
        {
            _loadSample = LoadSample::LoadSample1;
        }
    }
}

void
ReplicaGroupEntry::getLocatorAdapterInfo(
    LocatorAdapterInfoSeq& adapters,
    int& nReplicas,
    bool& replicaGroup,
    bool& roundRobin,
    string& filter,
    const set<string>& excludes)
{
    vector<shared_ptr<ServerAdapterEntry>> replicas;
    bool adaptive = false;
    LoadSample loadSample = LoadSample::LoadSample1;
    {
        unique_lock lock(_mutex);
        replicaGroup = true;
        roundRobin = false;
        filter = _filter;
        nReplicas = _loadBalancingNReplicas > 0 ? _loadBalancingNReplicas : static_cast<int>(_replicas.size());

        if (_replicas.empty())
        {
            return;
        }

        replicas.reserve(_replicas.size());
        if (dynamic_pointer_cast<RoundRobinLoadBalancingPolicy>(_loadBalancing))
        {
            // Serialize round-robin requests
            _condVar.wait(lock, [this] { return !_requestInProgress; });
            _requestInProgress = true;
            for (size_t i = 0; i < _replicas.size(); ++i)
            {
                replicas.push_back(_replicas[(static_cast<size_t>(_lastReplica) + i) % _replicas.size()]);
            }
            _lastReplica = (_lastReplica + 1) % static_cast<int>(_replicas.size());
            roundRobin = true;
        }
        else if (dynamic_pointer_cast<AdaptiveLoadBalancingPolicy>(_loadBalancing))
        {
            replicas = _replicas;
            IceInternal::shuffle(replicas.begin(), replicas.end());
            loadSample = _loadSample;
            adaptive = true;
        }
        else if (dynamic_pointer_cast<OrderedLoadBalancingPolicy>(_loadBalancing))
        {
            replicas = _replicas;
            sort(
                replicas.begin(),
                replicas.end(),
                [](const auto& lhs, const auto& rhs) { return lhs->getPriority() < rhs->getPriority(); });
        }
        else if (dynamic_pointer_cast<RandomLoadBalancingPolicy>(_loadBalancing))
        {
            replicas = _replicas;
            IceInternal::shuffle(replicas.begin(), replicas.end());
        }
    }

    int unreachable = 0;
    bool synchronizing = false;
    try
    {
        if (adaptive)
        {
            //
            // This must be done outside the synchronization block since
            // the transform() might call and lock each server adapter
            // entry. We also can't sort directly as the load of each
            // server adapter is not stable so we first take a snapshot of
            // each adapter and sort the snapshot.
            //
            vector<pair<float, shared_ptr<ServerAdapterEntry>>> rl;
            transform(
                replicas.begin(),
                replicas.end(),
                back_inserter(rl),
                [loadSample](const auto& value) -> pair<float, shared_ptr<ServerAdapterEntry>>
                { return {value->getLeastLoadedNodeLoad(loadSample), value}; });
            sort(rl.begin(), rl.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
            replicas.clear();
            transform(rl.begin(), rl.end(), back_inserter(replicas), [](const auto& value) { return value.second; });
        }

        //
        // Retrieve the proxy of each adapter from the server. The adapter
        // might not exist anymore at this time or the node might not be
        // reachable.
        //
        set<string> emptyExcludes;
        bool firstUnreachable = true;
        for (const auto& replica : replicas)
        {
            if (!roundRobin || excludes.find(replica->getId()) == excludes.end())
            {
                try
                {
                    replica->getLocatorAdapterInfo(adapters);
                    firstUnreachable = false;
                }
                catch (const SynchronizationException&)
                {
                    synchronizing = true;
                }
                catch (const Ice::UserException&)
                {
                    if (firstUnreachable)
                    {
                        ++unreachable; // Count the number of un-reachable nodes.
                    }
                }
            }
        }
    }
    catch (const std::exception&)
    {
        if (roundRobin)
        {
            lock_guard lock(_mutex);
            assert(_requestInProgress);
            _requestInProgress = false;
            _condVar.notify_one();
        }
        throw;
    }

    if (roundRobin)
    {
        lock_guard lock(_mutex);
        assert(_requestInProgress);
        _requestInProgress = false;
        _condVar.notify_one();
        if (unreachable > 0)
        {
            _lastReplica = (_lastReplica + unreachable) % static_cast<int>(_replicas.size());
        }
    }

    if (adapters.empty() && synchronizing)
    {
        throw SynchronizationException(__FILE__, __LINE__);
    }
}

float
ReplicaGroupEntry::getLeastLoadedNodeLoad(LoadSample loadSample) const
{
    vector<shared_ptr<ServerAdapterEntry>> replicas;
    {
        lock_guard lock(_mutex);
        replicas = _replicas;
    }

    if (replicas.empty())
    {
        return 999.9f;
    }
    else if (replicas.size() == 1)
    {
        return replicas.back()->getLeastLoadedNodeLoad(loadSample);
    }
    else
    {
        IceInternal::shuffle(replicas.begin(), replicas.end());
        vector<pair<float, shared_ptr<ServerAdapterEntry>>> rl;
        transform(
            replicas.begin(),
            replicas.end(),
            back_inserter(rl),
            [loadSample](const auto& value) -> pair<float, shared_ptr<ServerAdapterEntry>>
            { return {value->getLeastLoadedNodeLoad(loadSample), value}; });
        return min_element(rl.begin(), rl.end(), [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; })
            ->first;
    }
}

AdapterInfoSeq
ReplicaGroupEntry::getAdapterInfoNoEndpoints() const
{
    //
    // This method is called with the database locked so we're sure
    // that no new adapters will be added or removed concurrently.
    //
    vector<shared_ptr<ServerAdapterEntry>> replicas;
    {
        lock_guard lock(_mutex);
        replicas = _replicas;
    }

    AdapterInfoSeq infos;
    for (const auto& replica : replicas)
    {
        AdapterInfoSeq infs = replica->getAdapterInfoNoEndpoints();
        assert(infs.size() == 1);
        infos.push_back(infs[0]);
    }
    return infos;
}

shared_ptr<GetAdapterInfoResult>
ReplicaGroupEntry::getAdapterInfoAsync() const
{
    auto result = make_shared<GetAdapterInfoResult>();
    vector<shared_ptr<ServerAdapterEntry>> replicas;
    {
        lock_guard lock(_mutex);
        replicas = _replicas;
    }
    for (const auto& replica : replicas)
    {
        result->add(replica.get());
    }
    return result;
}

bool
ReplicaGroupEntry::hasAdaptersFromOtherApplications() const
{
    vector<shared_ptr<ServerAdapterEntry>> replicas;
    {
        lock_guard lock(_mutex);
        replicas = _replicas;
    }

    AdapterInfoSeq infos;
    for (const auto& replica : replicas)
    {
        if (replica->getApplication() != _application)
        {
            return true;
        }
    }
    return false;
}
