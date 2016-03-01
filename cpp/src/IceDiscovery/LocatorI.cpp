// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceDiscovery/LocatorI.h>
#include <IceDiscovery/LookupI.h>

#include <Ice/LocalException.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

LocatorRegistryI::LocatorRegistryI(const Ice::CommunicatorPtr& com) :
    _wellKnownProxy(com->stringToProxy("p")->ice_locator(0)->ice_router(0)->ice_collocationOptimized(true))
{
}

void 
LocatorRegistryI::setAdapterDirectProxy_async(const AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb, 
                                              const std::string& adapterId, 
                                              const ObjectPrx& proxy, 
                                              const Current&)
{
    Lock sync(*this);
    if(proxy)
    {
        _adapters[adapterId] = proxy;
    }
    else
    {
        _adapters.erase(adapterId);
    }
    cb->ice_response();
}

void
LocatorRegistryI::setReplicatedAdapterDirectProxy_async(
    const AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const std::string& adapterId,
    const std::string& replicaGroupId,
    const ObjectPrx& proxy, 
    const Current&)
{
    Lock sync(*this);
    if(proxy)
    {
        _adapters[adapterId] = proxy;
        map<string, set<string> >::iterator p = _replicaGroups.find(replicaGroupId);
        if(p == _replicaGroups.end())
        {
            p = _replicaGroups.insert(make_pair(replicaGroupId, set<string>())).first;
        }
        p->second.insert(adapterId);
    }
    else
    {
        _adapters.erase(adapterId);
        map<string, set<string> >::iterator p = _replicaGroups.find(replicaGroupId);
        if(p != _replicaGroups.end())
        {
            p->second.erase(adapterId);
            if(p->second.empty())
            {
                _replicaGroups.erase(p);
            }
        }
    }
    cb->ice_response();
}

void
LocatorRegistryI::setServerProcessProxy_async(const AMD_LocatorRegistry_setServerProcessProxyPtr& cb, 
                                              const std::string&, 
                                              const ProcessPrx&,
                                              const Current&)
{
    cb->ice_response();
}

Ice::ObjectPrx 
LocatorRegistryI::findObject(const Ice::Identity& id) const
{
    Lock sync(*this);
    if(id.name.empty())
    {
        return 0;
    }

    Ice::ObjectPrx prx = _wellKnownProxy->ice_identity(id);

    vector<string> adapterIds;
    for(map<string, set<string> >::const_iterator p = _replicaGroups.begin(); p != _replicaGroups.end(); ++p)
    {
        try
        {
            prx->ice_adapterId(p->first)->ice_ping();
            adapterIds.push_back(p->first);
        }
        catch(const Ice::Exception&)
        {
            // Ignore
        }
    }

    if(adapterIds.empty())
    {
        for(map<string, Ice::ObjectPrx>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
        {
            try
            {
                prx->ice_adapterId(p->first)->ice_ping();
                adapterIds.push_back(p->first);
            }
            catch(const Ice::Exception&)
            {
                // Ignore
            }
        }
    }

    if(adapterIds.empty())
    {
        return 0;
    }

    random_shuffle(adapterIds.begin(), adapterIds.end());
    return prx->ice_adapterId(adapterIds[0]);
}

Ice::ObjectPrx 
LocatorRegistryI::findAdapter(const string& adapterId, bool& isReplicaGroup) const
{
    Lock sync(*this);

    map<string, Ice::ObjectPrx>::const_iterator p = _adapters.find(adapterId);
    if(p != _adapters.end())
    {
        isReplicaGroup = false;
        return p->second;
    }

    map<string, set<string> >::const_iterator q = _replicaGroups.find(adapterId);
    if(q != _replicaGroups.end())
    {
        Ice::EndpointSeq endpoints;
        Ice::ObjectPrx prx;
        for(set<string>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
        {
            map<string, Ice::ObjectPrx>::const_iterator s = _adapters.find(*r);
            if(s == _adapters.end())
            {
                continue; // TODO: Inconsistency
            }

            if(!prx)
            {
                prx = s->second;
            }

            Ice::EndpointSeq endpts = s->second->ice_getEndpoints();
            copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
        }

        if(prx)
        {
            isReplicaGroup = true;
            return prx->ice_endpoints(endpoints);
        }
    }
    
    isReplicaGroup = false;
    return 0;
}

LocatorI::LocatorI(const LookupIPtr& lookup, const LocatorRegistryPrx& registry) : _lookup(lookup), _registry(registry)
{
}

void 
LocatorI::findObjectById_async(const AMD_Locator_findObjectByIdPtr& cb, 
                               const Identity& id, 
                               const Current&) const
{
    _lookup->findObject(cb, id);
}

void 
LocatorI::findAdapterById_async(const AMD_Locator_findAdapterByIdPtr& cb, 
                                const std::string& adapterId, 
                                const Current&) const
{
    _lookup->findAdapter(cb, adapterId);
}

LocatorRegistryPrx 
LocatorI::getRegistry(const Current&) const
{
    return _registry;
}

