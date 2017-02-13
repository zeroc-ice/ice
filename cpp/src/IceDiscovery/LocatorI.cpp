// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

LocatorRegistryI::LocatorRegistryI(const Ice::CommunicatorPtr& com) :
    _wellKnownProxy(com->stringToProxy("p")->ice_locator(0)->ice_router(0)->ice_collocationOptimized(true))
{
}


#ifdef ICE_CPP11_MAPPING
void
LocatorRegistryI::setAdapterDirectProxyAsync(string adapterId,
                                              shared_ptr<ObjectPrx> proxy,
                                              function<void()> response,
                                              function<void(exception_ptr)>,
                                              const Ice::Current&)
#else
void
LocatorRegistryI::setAdapterDirectProxy_async(const AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
                                              const std::string& adapterId,
                                              const ObjectPrxPtr& proxy,
                                              const Current&)
#endif
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
#ifdef ICE_CPP11_MAPPING
    response();
#else
    cb->ice_response();
#endif
}

#ifdef ICE_CPP11_MAPPING
void
LocatorRegistryI::setReplicatedAdapterDirectProxyAsync(string adapterId,
                                                        string replicaGroupId,
                                                        shared_ptr<ObjectPrx> proxy,
                                                        function<void()> response,
                                                        function<void(exception_ptr)>,
                                                        const Ice::Current&)
#else
void
LocatorRegistryI::setReplicatedAdapterDirectProxy_async(
    const AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const std::string& adapterId,
    const std::string& replicaGroupId,
    const ObjectPrxPtr& proxy,
    const Current&)
#endif
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
#ifdef ICE_CPP11_MAPPING
    response();
#else
    cb->ice_response();
#endif
}

#ifdef ICE_CPP11_MAPPING
void
LocatorRegistryI::setServerProcessProxyAsync(string,
                                              shared_ptr<ProcessPrx>,
                                              function<void()> response,
                                              function<void(exception_ptr)>,
                                              const Ice::Current&)
{
    response();
}
#else
void
LocatorRegistryI::setServerProcessProxy_async(const AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                              const std::string&,
                                              const ProcessPrxPtr&,
                                              const Current&)
{
    cb->ice_response();
}
#endif

Ice::ObjectPrxPtr
LocatorRegistryI::findObject(const Ice::Identity& id) const
{
    Lock sync(*this);
    if(id.name.empty())
    {
        return 0;
    }

    Ice::ObjectPrxPtr prx = _wellKnownProxy->ice_identity(id);

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
        for(map<string, Ice::ObjectPrxPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
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

Ice::ObjectPrxPtr
LocatorRegistryI::findAdapter(const string& adapterId, bool& isReplicaGroup) const
{
    Lock sync(*this);

    map<string, Ice::ObjectPrxPtr>::const_iterator p = _adapters.find(adapterId);
    if(p != _adapters.end())
    {
        isReplicaGroup = false;
        return p->second;
    }

    map<string, set<string> >::const_iterator q = _replicaGroups.find(adapterId);
    if(q != _replicaGroups.end())
    {
        Ice::EndpointSeq endpoints;
        Ice::ObjectPrxPtr prx;
        for(set<string>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
        {
            map<string, Ice::ObjectPrxPtr>::const_iterator s = _adapters.find(*r);
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

LocatorI::LocatorI(const LookupIPtr& lookup, const LocatorRegistryPrxPtr& registry) : _lookup(lookup), _registry(registry)
{
}

#ifdef ICE_CPP11_MAPPING
void
LocatorI::findObjectByIdAsync(Ice::Identity id,
                               function<void(const shared_ptr<ObjectPrx>&)> response,
                               function<void(exception_ptr)>,
                               const Ice::Current&) const
{
    _lookup->findObject(response, id);
}

void
LocatorI::findAdapterByIdAsync(string adapterId,
                                function<void(const shared_ptr<ObjectPrx>&)> response,
                                function<void(exception_ptr)>,
                                const Ice::Current&) const
{
    _lookup->findAdapter(response, adapterId);
}
#else
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
#endif

LocatorRegistryPrxPtr
LocatorI::getRegistry(const Current&) const
{
    return _registry;
}
