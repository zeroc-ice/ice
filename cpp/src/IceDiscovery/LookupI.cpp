// Copyright (c) ZeroC, Inc.

#include "LookupI.h"
#include "Ice/Communicator.h"
#include "Ice/Connection.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/UUID.h"

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

IceDiscovery::Request::Request(LookupIPtr lookup, int retryCount)
    : _lookup(std::move(lookup)),
      _requestId(Ice::generateUUID()),
      _retryCount(retryCount)
{
}

bool
IceDiscovery::Request::retry()
{
    return --_retryCount >= 0;
}

void
IceDiscovery::Request::invoke(const string& domainId, const vector<pair<LookupPrx, LookupReplyPrx>>& lookups)
{
    _lookupCount = lookups.size();
    _failureCount = 0;
    Ice::Identity id;
    id.name = _requestId;

    for (const auto& p : lookups)
    {
        invokeWithLookup(domainId, p.first, p.second->ice_identity<LookupReplyPrx>(id));
    }
}

bool
IceDiscovery::Request::exception()
{
    // If all the invocations on all the lookup proxies failed, report it to the locator.
    if (++_failureCount == _lookupCount)
    {
        finished(nullopt);
        return true;
    }
    return false;
}

string
IceDiscovery::Request::getRequestId() const
{
    return _requestId;
}

AdapterRequest::AdapterRequest(const LookupIPtr& lookup, const std::string& adapterId, int retryCount)
    : RequestT<std::string, AdapterCB>(lookup, adapterId, retryCount),
      _start(chrono::steady_clock::now()),
      _latency(chrono::nanoseconds::zero())
{
}

bool
AdapterRequest::retry()
{
    return _proxies.empty() && --_retryCount >= 0;
}

bool
AdapterRequest::response(const ObjectPrx& proxy, bool isReplicaGroup)
{
    if (isReplicaGroup)
    {
        if (_latency == chrono::nanoseconds::zero())
        {
            _latency = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - _start) *
                       _lookup->latencyMultiplier();
            _lookup->timer()->cancel(shared_from_this());
            _lookup->timer()->schedule(shared_from_this(), _latency);
        }
        _proxies.insert(proxy);
        return false;
    }
    finished(proxy);
    return true;
}

void
AdapterRequest::finished(const optional<ObjectPrx>& proxy)
{
    if (proxy || _proxies.empty())
    {
        RequestT<string, AdapterCB>::finished(proxy);
    }
    else if (_proxies.size() == 1)
    {
        RequestT<string, AdapterCB>::finished(*_proxies.begin());
    }
    else
    {
        EndpointSeq endpoints;
        optional<ObjectPrx> prx;
        for (const auto& p : _proxies)
        {
            if (!prx)
            {
                prx = p;
            }
            Ice::EndpointSeq endpts = p->ice_getEndpoints();
            copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
        }
        RequestT<string, AdapterCB>::finished(prx->ice_endpoints(endpoints));
    }
}

void
AdapterRequest::invokeWithLookup(const string& domainId, const LookupPrx& lookup, const LookupReplyPrx& lookupReply)
{
    lookup->findAdapterByIdAsync(
        domainId,
        _id,
        lookupReply,
        nullptr,
        [self = shared_from_this()](exception_ptr ex) { self->_lookup->adapterRequestException(self, ex); });
}

void
AdapterRequest::runTimerTask()
{
    _lookup->adapterRequestTimedOut(shared_from_this());
}

ObjectRequest::ObjectRequest(const LookupIPtr& lookup, const Ice::Identity& id, int retryCount)
    : RequestT<Ice::Identity, ObjectCB>(lookup, id, retryCount)
{
}

void
ObjectRequest::response(const ObjectPrx& proxy)
{
    finished(proxy);
}

void
ObjectRequest::invokeWithLookup(const string& domainId, const LookupPrx& lookup, const LookupReplyPrx& lookupReply)
{
    lookup->findObjectByIdAsync(
        domainId,
        _id,
        lookupReply,
        nullptr,
        [self = shared_from_this()](exception_ptr ex) { self->_lookup->objectRequestException(self, ex); });
}

void
ObjectRequest::runTimerTask()
{
    _lookup->objectRequestTimedOut(shared_from_this());
}

LookupI::LookupI(LocatorRegistryIPtr registry, const LookupPrx& lookup, const Ice::PropertiesPtr& properties)
    : _registry(std::move(registry)),
      _lookup(lookup),
      _timeout(chrono::milliseconds(properties->getIcePropertyAsInt("IceDiscovery.Timeout"))),
      _retryCount(properties->getIcePropertyAsInt("IceDiscovery.RetryCount")),
      _latencyMultiplier(properties->getIcePropertyAsInt("IceDiscovery.LatencyMultiplier")),
      _domainId(properties->getIceProperty("IceDiscovery.DomainId")),
      _timer(IceInternal::getInstanceTimer(lookup->ice_getCommunicator()))
{
}

void
LookupI::destroy()
{
    lock_guard lock(_mutex);
    for (const auto& objectRequest : _objectRequests)
    {
        objectRequest.second->finished(nullopt);
        _timer->cancel(objectRequest.second);
    }
    _objectRequests.clear();

    for (const auto& adapterRequest : _adapterRequests)
    {
        adapterRequest.second->finished(nullopt);
        _timer->cancel(adapterRequest.second);
    }
    _adapterRequests.clear();
}

void
LookupI::setLookupReply(const LookupReplyPrx& lookupReply)
{
    // This method is only called once from PluginI::initialize.
    assert(_lookups.empty());
    // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast datagram on each
    // endpoint.
    for (const auto& lookupEndpoint : _lookup->ice_getEndpoints())
    {
        // Use a lookup reply proxy whose address matches the interface used to send multicast datagrams.
        LookupReplyPrx reply = lookupReply;
        auto info = dynamic_pointer_cast<UDPEndpointInfo>(lookupEndpoint->getInfo());
        if (info && !info->mcastInterface.empty())
        {
            for (const auto& replyEndpoint : lookupReply->ice_getEndpoints())
            {
                auto r = dynamic_pointer_cast<IPEndpointInfo>(replyEndpoint->getInfo());
                if (r && r->host == info->mcastInterface)
                {
                    reply = reply->ice_endpoints(EndpointSeq{replyEndpoint});
                    break;
                }
            }
        }

        _lookups.emplace_back(_lookup->ice_endpoints(EndpointSeq{lookupEndpoint}), reply);
    }
    assert(!_lookups.empty());
}

void
LookupI::findObjectById(string domainId, Ice::Identity id, optional<LookupReplyPrx> reply, const Ice::Current&)
{
    if (domainId != _domainId)
    {
        return; // Ignore.
    }

    optional<Ice::ObjectPrx> proxy = _registry->findObject(id);
    if (proxy)
    {
        // Reply to the multicast request using the given proxy.
        try
        {
            reply->foundObjectByIdAsync(id, proxy, nullptr); // ignore response
        }
        catch (const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
LookupI::findAdapterById(string domainId, string adapterId, optional<LookupReplyPrx> reply, const Ice::Current&)
{
    if (domainId != _domainId)
    {
        return; // Ignore.
    }

    bool isReplicaGroup;
    optional<Ice::ObjectPrx> proxy = _registry->findAdapter(adapterId, isReplicaGroup);
    if (proxy)
    {
        //
        // Reply to the multicast request using the given proxy.
        //
        try
        {
            reply->foundAdapterByIdAsync(adapterId, proxy, isReplicaGroup, nullptr); // ignore response
        }
        catch (const Ice::LocalException&)
        {
            // Ignore.
        }
    }
}

void
LookupI::findObject(const ObjectCB& cb, const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    auto p = _objectRequests.find(id);
    if (p == _objectRequests.end())
    {
        p = _objectRequests
                .insert(make_pair(id, make_shared<ObjectRequest>(LookupIPtr(shared_from_this()), id, _retryCount)))
                .first;
    }

    if (p->second->addCallback(cb))
    {
        try
        {
            p->second->invoke(_domainId, _lookups);
            _timer->schedule(p->second, _timeout);
        }
        catch (const Ice::LocalException&)
        {
            p->second->finished(nullopt);
            _objectRequests.erase(p);
        }
    }
}

void
LookupI::findAdapter(const AdapterCB& cb, const std::string& adapterId)
{
    lock_guard lock(_mutex);
    auto p = _adapterRequests.find(adapterId);
    if (p == _adapterRequests.end())
    {
        p = _adapterRequests
                .insert(make_pair(
                    adapterId,
                    make_shared<AdapterRequest>(LookupIPtr(shared_from_this()), adapterId, _retryCount)))
                .first;
    }

    if (p->second->addCallback(cb))
    {
        try
        {
            p->second->invoke(_domainId, _lookups);
            _timer->schedule(p->second, _timeout);
        }
        catch (const Ice::LocalException&)
        {
            p->second->finished(nullopt);
            _adapterRequests.erase(p);
        }
    }
}

void
LookupI::foundObject(const Ice::Identity& id, const string& requestId, const ObjectPrx& proxy)
{
    lock_guard lock(_mutex);
    auto p = _objectRequests.find(id);
    // Ignore responses from old requests
    if (p != _objectRequests.end() && p->second->getRequestId() == requestId)
    {
        p->second->response(proxy);
        _timer->cancel(p->second);
        _objectRequests.erase(p);
    }
}

void
LookupI::foundAdapter(const string& adapterId, const string& requestId, const ObjectPrx& proxy, bool isReplicaGroup)
{
    lock_guard lock(_mutex);

    auto p = _adapterRequests.find(adapterId);
    if (p != _adapterRequests.end() && p->second->getRequestId() == requestId) // Ignore responses from old requests
    {
        if (p->second->response(proxy, isReplicaGroup))
        {
            _timer->cancel(p->second);
            _adapterRequests.erase(p);
        }
    }
}

void
LookupI::objectRequestTimedOut(const ObjectRequestPtr& request)
{
    lock_guard lock(_mutex);
    auto p = _objectRequests.find(request->getId());
    if (p == _objectRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if (request->retry())
    {
        try
        {
            request->invoke(_domainId, _lookups);
            _timer->schedule(request, _timeout);
            return;
        }
        catch (const Ice::LocalException&)
        {
        }
    }

    request->finished(nullopt);
    _objectRequests.erase(p);
    _timer->cancel(request);
}

void
LookupI::adapterRequestException(const AdapterRequestPtr& request, exception_ptr ex)
{
    lock_guard lock(_mutex);
    auto p = _adapterRequests.find(request->getId());
    if (p == _adapterRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if (request->exception())
    {
        if (_warnOnce)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Warning warn(_lookup->ice_getCommunicator()->getLogger());
                warn << "failed to lookup adapter '" << p->first << "' with lookup proxy '" << _lookup << "':\n" << e;
                _warnOnce = false;
            }
        }
        _timer->cancel(request);
        _adapterRequests.erase(p);
    }
}

void
LookupI::adapterRequestTimedOut(const AdapterRequestPtr& request)
{
    lock_guard lock(_mutex);
    auto p = _adapterRequests.find(request->getId());
    if (p == _adapterRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if (request->retry())
    {
        try
        {
            request->invoke(_domainId, _lookups);
            _timer->schedule(request, _timeout);
            return;
        }
        catch (const Ice::LocalException&)
        {
        }
    }

    request->finished(nullopt);
    _adapterRequests.erase(p);
    _timer->cancel(request);
}

void
LookupI::objectRequestException(const ObjectRequestPtr& request, exception_ptr ex)
{
    lock_guard lock(_mutex);
    auto p = _objectRequests.find(request->getId());
    if (p == _objectRequests.end() || p->second.get() != request.get())
    {
        return;
    }

    if (request->exception())
    {
        if (_warnOnce)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Warning warn(_lookup->ice_getCommunicator()->getLogger());
                string id = _lookup->ice_getCommunicator()->identityToString(p->first);
                warn << "failed to lookup object '" << id << "' with lookup proxy '" << _lookup << "':\n" << e;
                _warnOnce = false;
            }
        }
        _timer->cancel(request);
        _objectRequests.erase(p);
    }
}

LookupReplyI::LookupReplyI(LookupIPtr lookup) : _lookup(std::move(lookup)) {}

void
LookupReplyI::foundObjectById(Identity id, optional<ObjectPrx> proxy, const Current& current)
{
    checkNotNull(proxy, __FILE__, __LINE__, current);
    _lookup->foundObject(id, current.id.name, *proxy);
}

void
LookupReplyI::foundAdapterById(string adapterId, optional<ObjectPrx> proxy, bool isReplicaGroup, const Current& current)
{
    checkNotNull(proxy, __FILE__, __LINE__, current);
    _lookup->foundAdapter(adapterId, current.id.name, *proxy, isReplicaGroup);
}
