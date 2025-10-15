// Copyright (c) ZeroC, Inc.

#include "LocatorI.h"
#include "../Ice/Random.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/ObjectAdapter.h"
#include "LookupI.h"

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

LocatorRegistryI::LocatorRegistryI(const Ice::CommunicatorPtr& communicator)
    : _wellKnownProxy{
          ObjectPrx{communicator, "dummy"}.ice_locator(nullopt).ice_router(nullopt).ice_collocationOptimized(true)}
{
}

void
LocatorRegistryI::setAdapterDirectProxy(string adapterId, optional<ObjectPrx> proxy, const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (proxy)
    {
        _adapters.insert({adapterId, std::move(*proxy)});
    }
    else
    {
        _adapters.erase(adapterId);
    }
}

void
LocatorRegistryI::setReplicatedAdapterDirectProxy(
    string adapterId,
    string replicaGroupId,
    optional<ObjectPrx> proxy,
    const Ice::Current&)
{
    lock_guard lock(_mutex);
    if (proxy)
    {
        _adapters.insert({adapterId, std::move(*proxy)});
        auto p = _replicaGroups.find(replicaGroupId);
        if (p == _replicaGroups.end())
        {
            p = _replicaGroups.insert(make_pair(replicaGroupId, set<string>())).first;
        }
        p->second.insert(adapterId);
    }
    else
    {
        _adapters.erase(adapterId);
        auto p = _replicaGroups.find(replicaGroupId);
        if (p != _replicaGroups.end())
        {
            p->second.erase(adapterId);
            if (p->second.empty())
            {
                _replicaGroups.erase(p);
            }
        }
    }
}

void
LocatorRegistryI::setServerProcessProxy(string, optional<ProcessPrx>, const Ice::Current&)
{
}

optional<Ice::ObjectPrx>
LocatorRegistryI::findObject(const Ice::Identity& id) const
{
    lock_guard lock(_mutex);
    if (id.name.empty())
    {
        return nullopt;
    }

    Ice::ObjectPrx prx = _wellKnownProxy->ice_identity(id);

    vector<string> adapterIds;
    for (const auto& [adapterId, _] : _replicaGroups)
    {
        try
        {
            prx->ice_adapterId(adapterId)->ice_ping();
            adapterIds.push_back(adapterId);
        }
        catch (const Ice::Exception&)
        {
            // Ignore
        }
    }

    if (adapterIds.empty())
    {
        for (const auto& [adapterId, _] : _adapters)
        {
            try
            {
                prx->ice_adapterId(adapterId)->ice_ping();
                adapterIds.push_back(adapterId);
            }
            catch (const Ice::Exception&)
            {
                // Ignore
            }
        }
    }

    if (adapterIds.empty())
    {
        return nullopt;
    }

    IceInternal::shuffle(adapterIds.begin(), adapterIds.end());
    return prx->ice_adapterId(adapterIds[0]);
}

optional<Ice::ObjectPrx>
LocatorRegistryI::findAdapter(const string& adapterId, bool& isReplicaGroup) const
{
    lock_guard lock(_mutex);

    auto p = _adapters.find(adapterId);
    if (p != _adapters.end())
    {
        isReplicaGroup = false;
        return p->second;
    }

    auto q = _replicaGroups.find(adapterId);
    if (q != _replicaGroups.end())
    {
        Ice::EndpointSeq endpoints;
        optional<Ice::ObjectPrx> prx;
        for (const auto& r : q->second)
        {
            auto s = _adapters.find(r);
            if (s == _adapters.end())
            {
                continue; // TODO: Inconsistency
            }

            if (!prx)
            {
                prx = s->second;
            }

            Ice::EndpointSeq endpts = s->second->ice_getEndpoints();
            copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
        }

        if (prx)
        {
            isReplicaGroup = true;
            return prx->ice_endpoints(endpoints);
        }
    }

    isReplicaGroup = false;
    return nullopt;
}

LocatorI::LocatorI(LookupIPtr lookup, LocatorRegistryPrx registry)
    : _lookup(std::move(lookup)),
      _registry(std::move(registry))
{
}

void
LocatorI::findObjectByIdAsync(
    Ice::Identity id,
    function<void(const optional<ObjectPrx>&)> response,
    function<void(exception_ptr)> ex,
    const Ice::Current&) const
{
    _lookup->findObject(make_pair(response, ex), id);
}

void
LocatorI::findAdapterByIdAsync(
    string adapterId,
    function<void(const optional<ObjectPrx>&)> response,
    function<void(exception_ptr)> ex,
    const Ice::Current&) const
{
    _lookup->findAdapter(make_pair(response, ex), adapterId);
}

void
LocatorI::getRegistryAsync(
    function<void(const optional<LocatorRegistryPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&) const
{
    response(_registry);
}
