// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Connection.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Initialize.h>

#include <IceDiscovery/LookupI.h>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

#ifndef ICE_CPP11_MAPPING
IceDiscovery::Request::Request(const LookupIPtr& lookup, int retryCount) : _lookup(lookup), _nRetry(retryCount)
{
}

bool
IceDiscovery::Request::retry()
{
    return --_nRetry >= 0;
}
#endif

bool
AdapterRequest::retry()
{
    return _proxies.empty() && --_nRetry >= 0;
}

bool
AdapterRequest::response(const Ice::ObjectPrxPtr& proxy, bool isReplicaGroup)
{
    if(isReplicaGroup)
    {
        if(_latency == IceUtil::Time())
        {
            _latency = (IceUtil::Time::now() - _start) * _lookup->latencyMultiplier();
            _lookup->timer()->cancel(ICE_SHARED_FROM_THIS);
            _lookup->timer()->schedule(ICE_SHARED_FROM_THIS, _latency);
        }
        _proxies.push_back(proxy);
        return false;
    }
    finished(proxy);
    return true;
}

#ifdef ICE_CPP11_MAPPING
void
AdapterRequest::finished(const Ice::ObjectPrxPtr& proxy)
{
    if(proxy || _proxies.empty())
    {
        Request<string>::finished(proxy);
        return;
    }
    else if(_proxies.size() == 1)
    {
        Request<string>::finished(_proxies[0]);
        return;
    }

    EndpointSeq endpoints;
    shared_ptr<ObjectPrx> prx;
    for(vector<shared_ptr<ObjectPrx>>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
    {
        if(!prx)
        {
            prx = *p;
        }
        Ice::EndpointSeq endpts = (*p)->ice_getEndpoints();
        copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
    }
    Request<string>::finished(prx->ice_endpoints(endpoints));
}
#else
void
AdapterRequest::finished(const Ice::ObjectPrxPtr& proxy)
{
    if(proxy || _proxies.empty())
    {
        RequestT<std::string, Ice::AMD_Locator_findAdapterByIdPtr>::finished(proxy);
        return;
    }
    else if(_proxies.size() == 1)
    {
        RequestT<std::string, Ice::AMD_Locator_findAdapterByIdPtr>::finished(_proxies[0]);
        return;
    }

    Ice::EndpointSeq endpoints;
    Ice::ObjectPrxPtr prx;
    for(vector<Ice::ObjectPrxPtr>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
    {
        if(!prx)
        {
            prx = *p;
        }
        Ice::EndpointSeq endpts = (*p)->ice_getEndpoints();
        copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
    }
    RequestT<std::string, Ice::AMD_Locator_findAdapterByIdPtr>::finished(prx->ice_endpoints(endpoints));
}
#endif

void
AdapterRequest::runTimerTask()
{
    _lookup->adapterRequestTimedOut(shared_from_this());
}

void
ObjectRequest::response(const Ice::ObjectPrxPtr& proxy)
{
    finished(proxy);
}

void
ObjectRequest::runTimerTask()
{
    _lookup->objectRequestTimedOut(shared_from_this());
}

LookupI::LookupI(const LocatorRegistryIPtr& registry, const LookupPrxPtr& lookup, const Ice::PropertiesPtr& properties) :
    _registry(registry),
    _lookup(lookup),
    _timeout(IceUtil::Time::milliSeconds(properties->getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300))),
    _retryCount(properties->getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3)),
    _latencyMultiplier(properties->getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1)),
    _domainId(properties->getProperty("IceDiscovery.DomainId")),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator()))
{
}

LookupI::~LookupI()
{
}

void
LookupI::destroy()
{
    Lock sync(*this);
    for(map<Identity, ObjectRequestPtr>::const_iterator p = _objectRequests.begin(); p != _objectRequests.end(); ++p)
    {
        p->second->finished(0);
        _timer->cancel(p->second);
    }

    for(map<string, AdapterRequestPtr>::const_iterator p = _adapterRequests.begin(); p != _adapterRequests.end(); ++p)
    {
        p->second->finished(0);
        _timer->cancel(p->second);
    }
    _adapterRequests.clear();
}

void
LookupI::setLookupReply(const LookupReplyPrxPtr& lookupReply)
{
    _lookupReply = lookupReply;
}

void
#ifdef ICE_CPP11_MAPPING
LookupI::findObjectById(string domainId, Ice::Identity id, shared_ptr<IceDiscovery::LookupReplyPrx> reply,
                        const Ice::Current&)
#else
LookupI::findObjectById(const string& domainId, const Ice::Identity& id, const IceDiscovery::LookupReplyPrx& reply,
                        const Ice::Current&)
#endif
{
    if(domainId != _domainId)
    {
        return; // Ignore.
    }

    Ice::ObjectPrxPtr proxy = _registry->findObject(id);
    if(proxy)
    {
        //
        // Reply to the mulicast request using the given proxy.
        //
        try
        {
#ifdef ICE_CPP11_MAPPING
            reply->foundObjectByIdAsync(id, proxy);
#else
            reply->begin_foundObjectById(id, proxy);
#endif
        }
        catch(const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
#ifdef ICE_CPP11_MAPPING
LookupI::findAdapterById(string domainId, string adapterId, shared_ptr<IceDiscovery::LookupReplyPrx> reply,
                         const Ice::Current&)
#else
LookupI::findAdapterById(const string& domainId, const string& adapterId, const IceDiscovery::LookupReplyPrxPtr& reply,
                         const Ice::Current&)
#endif
{
    if(domainId != _domainId)
    {
        return; // Ignore.
    }

    bool isReplicaGroup;
    Ice::ObjectPrxPtr proxy = _registry->findAdapter(adapterId, isReplicaGroup);
    if(proxy)
    {
        //
        // Reply to the multicast request using the given proxy.
        //
        try
        {
#ifdef ICE_CPP11_MAPPING
            reply->foundAdapterByIdAsync(adapterId, proxy, isReplicaGroup);
#else
            reply->begin_foundAdapterById(adapterId, proxy, isReplicaGroup);
#endif
        }
        catch(const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

#ifdef ICE_CPP11_MAPPING
void
LookupI::findObject(function<void (const shared_ptr<Ice::ObjectPrx>&)> response, const Ice::Identity& id)
{
    Lock sync(*this);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(id);
    if(p == _objectRequests.end())
    {
        p = _objectRequests.insert(make_pair(id, make_shared<ObjectRequest>(shared_from_this(), id, _retryCount))).first;
    }

    if(p->second->addCallback(response))
    {
        try
        {
            _lookup->findObjectByIdAsync(_domainId, id, _lookupReply);
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(nullptr);
            _objectRequests.erase(p);
        }
    }
}

void
LookupI::findAdapter(function<void (const shared_ptr<Ice::ObjectPrx>&)> response, const std::string& adapterId)
{
    Lock sync(*this);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(adapterId);
    if(p == _adapterRequests.end())
    {
        p = _adapterRequests.insert(make_pair(adapterId, make_shared<AdapterRequest>(shared_from_this(), adapterId, _retryCount))).first;
    }

    if(p->second->addCallback(response))
    {
        try
        {
            _lookup->findAdapterByIdAsync(_domainId, adapterId, _lookupReply);
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(0);
            _adapterRequests.erase(p);
        }
    }
}
#else
void
LookupI::findObject(const Ice::AMD_Locator_findObjectByIdPtr& cb, const Ice::Identity& id)
{
    Lock sync(*this);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(id);
    if(p == _objectRequests.end())
    {
        p = _objectRequests.insert(make_pair(id, new ObjectRequest(this, id, _retryCount))).first;
    }

    if(p->second->addCallback(cb))
    {
        try
        {
            _lookup->begin_findObjectById(_domainId, id, _lookupReply);
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(0);
            _objectRequests.erase(p);
        }
    }
}

void
LookupI::findAdapter(const Ice::AMD_Locator_findAdapterByIdPtr& cb, const std::string& adapterId)
{
    Lock sync(*this);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(adapterId);
    if(p == _adapterRequests.end())
    {
        p = _adapterRequests.insert(make_pair(adapterId, new AdapterRequest(this, adapterId, _retryCount))).first;
    }

    if(p->second->addCallback(cb))
    {
        try
        {
            _lookup->begin_findAdapterById(_domainId, adapterId, _lookupReply);
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(0);
            _adapterRequests.erase(p);
        }
    }
}
#endif

void
LookupI::foundObject(const Ice::Identity& id, const Ice::ObjectPrxPtr& proxy)
{
    Lock sync(*this);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(id);
    if(p == _objectRequests.end())
    {
        return;
    }

    p->second->response(proxy);
    _timer->cancel(p->second);
    _objectRequests.erase(p);
}

void
LookupI::foundAdapter(const std::string& adapterId, const Ice::ObjectPrxPtr& proxy, bool isReplicaGroup)
{
    Lock sync(*this);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(adapterId);
    if(p == _adapterRequests.end())
    {
        return;
    }

    if(p->second->response(proxy, isReplicaGroup))
    {
        _timer->cancel(p->second);
        _adapterRequests.erase(p);
    }
}

void
LookupI::objectRequestTimedOut(const ObjectRequestPtr& request)
{
    Lock sync(*this);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(request->getId());
    if(p == _objectRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if(request->retry())
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            _lookup->findObjectByIdAsync(_domainId, request->getId(), _lookupReply);
#else
            _lookup->begin_findObjectById(_domainId, request->getId(), _lookupReply);
#endif
            _timer->schedule(p->second, _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    request->finished(0);
    _objectRequests.erase(p);
    _timer->cancel(request);
}

void
LookupI::adapterRequestTimedOut(const AdapterRequestPtr& request)
{
    Lock sync(*this);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(request->getId());
    if(p == _adapterRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if(request->retry())
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            _lookup->findAdapterByIdAsync(_domainId, request->getId(), _lookupReply);
#else
            _lookup->begin_findAdapterById(_domainId, request->getId(), _lookupReply);
#endif
            _timer->schedule(p->second, _timeout);
            return;
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    request->finished(0);
    _adapterRequests.erase(p);
    _timer->cancel(request);
}

LookupReplyI::LookupReplyI(const LookupIPtr& lookup) : _lookup(lookup)
{
}

#ifdef ICE_CPP11_MAPPING
void
LookupReplyI::foundObjectById(Identity id, shared_ptr<ObjectPrx> proxy, const Current&)
{
    _lookup->foundObject(id, proxy);
}

void
LookupReplyI::foundAdapterById(string adapterId, shared_ptr<ObjectPrx> proxy, bool isReplicaGroup, const Current&)
{
    _lookup->foundAdapter(adapterId, proxy, isReplicaGroup);
}
#else
void
LookupReplyI::foundObjectById(const Identity& id, const ObjectPrxPtr& proxy, const Current&)
{
    _lookup->foundObject(id, proxy);
}

void
LookupReplyI::foundAdapterById(const string& adapterId, const ObjectPrxPtr& proxy, bool isReplicaGroup, const Current&)
{
    _lookup->foundAdapter(adapterId, proxy, isReplicaGroup);
}
#endif
