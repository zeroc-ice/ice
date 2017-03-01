// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <iterator>

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

void
AdapterRequest::finished(const ObjectPrxPtr& proxy)
{
    if(proxy || _proxies.empty())
    {
#ifdef ICE_CPP11_MAPPING
        Request<string>::finished(proxy);
#else
        RequestT<string, AMD_Locator_findAdapterByIdPtr>::finished(proxy);
#endif
        return;
    }
    else if(_proxies.size() == 1)
    {
#ifdef ICE_CPP11_MAPPING
        Request<string>::finished(_proxies[0]);
#else
        RequestT<string, AMD_Locator_findAdapterByIdPtr>::finished(_proxies[0]);
#endif
        return;
    }

    EndpointSeq endpoints;
    ObjectPrxPtr prx;
    for(vector<ObjectPrxPtr>::const_iterator p = _proxies.begin(); p != _proxies.end(); ++p)
    {
        if(!prx)
        {
            prx = *p;
        }
        Ice::EndpointSeq endpts = (*p)->ice_getEndpoints();
        copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
    }
#ifdef ICE_CPP11_MAPPING
    Request<string>::finished(prx->ice_endpoints(endpoints));
#else
    RequestT<string, AMD_Locator_findAdapterByIdPtr>::finished(prx->ice_endpoints(endpoints));
#endif
}

void
AdapterRequest::runTimerTask()
{
    _lookup->adapterRequestTimedOut(ICE_SHARED_FROM_THIS);
}

void
ObjectRequest::response(const Ice::ObjectPrxPtr& proxy)
{
    finished(proxy);
}

void
ObjectRequest::runTimerTask()
{
    _lookup->objectRequestTimedOut(ICE_SHARED_FROM_THIS);
}

LookupI::LookupI(const LocatorRegistryIPtr& registry, const LookupPrxPtr& lookup, const Ice::PropertiesPtr& properties) :
    _registry(registry),
    _timeout(IceUtil::Time::milliSeconds(properties->getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300))),
    _retryCount(properties->getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3)),
    _latencyMultiplier(properties->getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1)),
    _domainId(properties->getProperty("IceDiscovery.DomainId")),
    _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator()))
{
#ifndef ICE_CPP11_MAPPING
    __setNoDelete(true);
#endif
    try
    {
        // Ensure we can establish a connection to the multicast proxy
        lookup->ice_getConnection();
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "IceDiscovery is unable to establish a multicast connection:\n";
        os << "proxy = " << lookup << '\n';
        os << ex;
        throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    //
    // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
    // datagram on each endpoint.
    //
    EndpointSeq endpoints = lookup->ice_getEndpoints();
    for(vector<EndpointPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        try
        {
            EndpointSeq single;
            single.push_back(*p);
            LookupPrxPtr l = lookup->ice_endpoints(single);
            l->ice_getConnection();
            _lookup.push_back(make_pair(l, LookupReplyPrxPtr()));
        }
        catch(const Ice::LocalException&)
        {
        }
    }
    assert(!_lookup.empty());
#ifndef ICE_CPP11_MAPPING
    __setNoDelete(false);
#endif
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
    //
    // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
    //
    for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::iterator p = _lookup.begin(); p != _lookup.end(); ++p)
    {
        UDPEndpointInfoPtr info = ICE_DYNAMIC_CAST(UDPEndpointInfo, p->first->ice_getEndpoints()[0]->getInfo());
        if(info && !info->mcastInterface.empty())
        {
            EndpointSeq endpts = lookupReply->ice_getEndpoints();
            for(EndpointSeq::const_iterator q = endpts.begin(); q != endpts.end(); ++q)
            {
                IPEndpointInfoPtr r = ICE_DYNAMIC_CAST(IPEndpointInfo, (*q)->getInfo());
                if(r && r->host == info->mcastInterface)
                {
                    EndpointSeq single;
                    single.push_back(*q);
                    p->second = lookupReply->ice_endpoints(single);
                }
            }
        }

        if(!p->second)
        {
            p->second = lookupReply; // Fallback: just use the given lookup reply proxy if no matching endpoint found.
        }
    }
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

void
#ifdef ICE_CPP11_MAPPING
LookupI::findObject(function<void(const shared_ptr<Ice::ObjectPrx>&)> cb, const Ice::Identity& id)
#else
LookupI::findObject(const Ice::AMD_Locator_findObjectByIdPtr& cb, const Ice::Identity& id)
#endif
{
    Lock sync(*this);
    map<Ice::Identity, ObjectRequestPtr>::iterator p = _objectRequests.find(id);
    if(p == _objectRequests.end())
    {
        p = _objectRequests.insert(make_pair(id, ICE_MAKE_SHARED(ObjectRequest,
                                                                 ICE_SHARED_FROM_THIS,
                                                                 id,
                                                                 _retryCount))).first;
    }

    if(p->second->addCallback(cb))
    {
        try
        {
            for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookup.begin(); l != _lookup.end();
                ++l)
            {
#ifdef ICE_CPP11_MAPPING
                l->first->findObjectByIdAsync(_domainId, id, l->second);
#else
                l->first->begin_findObjectById(_domainId, id, l->second);
#endif
            }
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(ICE_NULLPTR);
            _objectRequests.erase(p);
        }
    }
}

void
#ifdef ICE_CPP11_MAPPING
LookupI::findAdapter(function<void(const shared_ptr<Ice::ObjectPrx>&)> cb, const std::string& adapterId)
#else
LookupI::findAdapter(const Ice::AMD_Locator_findAdapterByIdPtr& cb, const std::string& adapterId)
#endif
{
    Lock sync(*this);
    map<string, AdapterRequestPtr>::iterator p = _adapterRequests.find(adapterId);
    if(p == _adapterRequests.end())
    {
        p = _adapterRequests.insert(make_pair(adapterId, ICE_MAKE_SHARED(AdapterRequest,
                                                                         ICE_SHARED_FROM_THIS,
                                                                         adapterId,
                                                                         _retryCount))).first;
    }

    if(p->second->addCallback(cb))
    {
        try
        {
            for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookup.begin(); l != _lookup.end();
                ++l)
            {
#ifdef ICE_CPP11_MAPPING
                l->first->findAdapterByIdAsync(_domainId, adapterId, l->second);
#else
                l->first->begin_findAdapterById(_domainId, adapterId, l->second);
#endif
            }
            _timer->schedule(p->second, _timeout);
        }
        catch(const Ice::LocalException&)
        {
            p->second->finished(ICE_NULLPTR);
            _adapterRequests.erase(p);
        }
    }
}

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
            for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookup.begin(); l != _lookup.end();
                ++l)
            {
#ifdef ICE_CPP11_MAPPING
                l->first->findObjectByIdAsync(_domainId, request->getId(), l->second);
#else
                l->first->begin_findObjectById(_domainId, request->getId(), l->second);
#endif
            }
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
            for(vector<pair<LookupPrxPtr, LookupReplyPrxPtr> >::const_iterator l = _lookup.begin(); l != _lookup.end();
                ++l)
            {
#ifdef ICE_CPP11_MAPPING
                l->first->findAdapterByIdAsync(_domainId, request->getId(), l->second);
#else
                l->first->begin_findAdapterById(_domainId, request->getId(), l->second);
#endif
            }
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
