// Copyright (c) ZeroC, Inc.

#include "LocatorInfo.h"
#include "EndpointI.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Ice/ProxyFunctions.h"
#include "Instance.h"
#include "Reference.h"
#include "TraceLevels.h"

#include "DisableWarnings.h"

#include <algorithm>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    class ObjectRequest final : public LocatorInfo::Request, public std::enable_shared_from_this<ObjectRequest>
    {
    public:
        ObjectRequest(const LocatorInfoPtr& locatorInfo, const ReferencePtr& ref)
            : LocatorInfo::Request(locatorInfo, ref)
        {
            assert(ref->isWellKnown());
        }

        void send() override
        {
            try
            {
                LocatorInfo::RequestPtr request = shared_from_this();
                _locatorInfo->getLocator()->findObjectByIdAsync(
                    _reference->getIdentity(),
                    [request](const optional<ObjectPrx>& object) { request->response(object); },
                    [request](exception_ptr e) { request->exception(e); });
            }
            catch (const Ice::Exception&)
            {
                exception(current_exception());
            }
        }
    };

    class AdapterRequest final : public LocatorInfo::Request, public std::enable_shared_from_this<AdapterRequest>
    {
    public:
        AdapterRequest(const LocatorInfoPtr& locatorInfo, const ReferencePtr& ref)
            : LocatorInfo::Request(locatorInfo, ref)
        {
            assert(ref->isIndirect() && !ref->isWellKnown());
        }

        void send() override
        {
            try
            {
                LocatorInfo::RequestPtr request = shared_from_this();
                _locatorInfo->getLocator()->findAdapterByIdAsync(
                    _reference->getAdapterId(),
                    [request](const optional<ObjectPrx>& object) { request->response(object); },
                    [request](exception_ptr e) { request->exception(e); });
            }
            catch (const Ice::Exception&)
            {
                exception(current_exception());
            }
        }
    };
}

IceInternal::LocatorManager::LocatorManager(const Ice::PropertiesPtr& properties)
    : _background(properties->getIcePropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0),
      _tableHint(_table.end())
{
}

void
IceInternal::LocatorManager::destroy()
{
    lock_guard lock(_mutex);
    for (const auto& [_, locatorInfo] : _table)
    {
        locatorInfo->destroy();
    }
    _table.clear();
    _tableHint = _table.end();

    _locatorTables.clear();
}

LocatorInfoPtr
IceInternal::LocatorManager::get(const LocatorPrx& loc)
{
    LocatorPrx locator = loc->ice_locator(nullopt); // The locator can't be located.

    //
    // TODO: reap unused locator info objects?
    //

    lock_guard lock(_mutex);

    auto p = _table.end();

    if (_tableHint != _table.end())
    {
        if (_tableHint->first == locator)
        {
            p = _tableHint;
        }
    }

    if (p == _table.end())
    {
        p = _table.find(locator);
    }

    if (p == _table.end())
    {
        //
        // Rely on locator identity for the adapter table. We want to
        // have only one table per locator (not one per locator
        // proxy).
        //
        pair<Identity, EncodingVersion> locatorKey(locator->ice_getIdentity(), locator->ice_getEncodingVersion());
        auto t = _locatorTables.find(locatorKey);
        if (t == _locatorTables.end())
        {
            t = _locatorTables.insert(
                _locatorTables.begin(),
                pair<const pair<Identity, EncodingVersion>, LocatorTablePtr>(locatorKey, new LocatorTable()));
        }

        _tableHint = _table.insert(
            _tableHint,
            pair<const LocatorPrx, LocatorInfoPtr>(locator, new LocatorInfo(locator, t->second, _background)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

IceInternal::LocatorTable::LocatorTable() = default;

void
IceInternal::LocatorTable::clear()
{
    lock_guard lock(_mutex);

    _adapterEndpointsMap.clear();
    _objectMap.clear();
}

bool
IceInternal::LocatorTable::getAdapterEndpoints(
    const string& adapter,
    chrono::milliseconds ttl,
    vector<EndpointIPtr>& endpoints)
{
    if (ttl == 0ms) // No locator cache.
    {
        return false;
    }

    lock_guard lock(_mutex);

    auto p = _adapterEndpointsMap.find(adapter);

    if (p != _adapterEndpointsMap.end())
    {
        endpoints = p->second.second;
        return checkTTL(p->second.first, ttl);
    }
    return false;
}

void
IceInternal::LocatorTable::addAdapterEndpoints(const string& adapter, const vector<EndpointIPtr>& endpoints)
{
    lock_guard lock(_mutex);

    auto p = _adapterEndpointsMap.find(adapter);

    if (p != _adapterEndpointsMap.end())
    {
        p->second = make_pair(chrono::steady_clock::now(), endpoints);
    }
    else
    {
        _adapterEndpointsMap.insert(make_pair(adapter, make_pair(chrono::steady_clock::now(), endpoints)));
    }
}

vector<EndpointIPtr>
IceInternal::LocatorTable::removeAdapterEndpoints(const string& adapter)
{
    lock_guard lock(_mutex);

    auto p = _adapterEndpointsMap.find(adapter);
    if (p == _adapterEndpointsMap.end())
    {
        return {};
    }

    vector<EndpointIPtr> endpoints = p->second.second;

    _adapterEndpointsMap.erase(p);

    return endpoints;
}

bool
IceInternal::LocatorTable::getObjectReference(const Identity& id, chrono::milliseconds ttl, ReferencePtr& ref)
{
    if (ttl == 0ms) // No locator cache
    {
        return false;
    }

    lock_guard lock(_mutex);

    auto p = _objectMap.find(id);
    if (p != _objectMap.end())
    {
        ref = p->second.second;
        return checkTTL(p->second.first, ttl);
    }
    return false;
}

void
IceInternal::LocatorTable::addObjectReference(const Identity& id, const ReferencePtr& ref)
{
    lock_guard lock(_mutex);

    auto p = _objectMap.find(id);

    if (p != _objectMap.end())
    {
        p->second = make_pair(chrono::steady_clock::now(), ref);
    }
    else
    {
        _objectMap.insert(make_pair(id, make_pair(chrono::steady_clock::now(), ref)));
    }
}

ReferencePtr
IceInternal::LocatorTable::removeObjectReference(const Identity& id)
{
    lock_guard lock(_mutex);

    auto p = _objectMap.find(id);
    if (p == _objectMap.end())
    {
        return nullptr;
    }

    ReferencePtr ref = p->second.second;
    _objectMap.erase(p);
    return ref;
}

bool
IceInternal::LocatorTable::checkTTL(const chrono::steady_clock::time_point& time, chrono::milliseconds ttl) const
{
    assert(ttl != 0ms);
    if (ttl < 0ms) // TTL = infinite
    {
        return true;
    }
    else
    {
        return chrono::steady_clock::now() - time <= ttl;
    }
}

void
IceInternal::LocatorInfo::RequestCallback::response(const LocatorInfoPtr& locatorInfo, const optional<ObjectPrx>& proxy)
{
    vector<EndpointIPtr> endpoints;
    if (proxy)
    {
        ReferencePtr r = proxy->_getReference();
        if (_reference->isWellKnown() && !isSupported(_reference->getEncoding(), r->getEncoding()))
        {
            //
            // If a well-known proxy and the returned proxy encoding
            // isn't supported, we're done: there's no compatible
            // endpoint we can use.
            //
        }
        else if (!r->isIndirect())
        {
            endpoints = r->getEndpoints();
        }
        else if (_reference->isWellKnown() && !r->isWellKnown())
        {
            //
            // We're resolving the endpoints of a well-known object and the proxy returned
            // by the locator is an indirect proxy. We now need to resolve the endpoints
            // of this indirect proxy.
            //
            if (_reference->getInstance()->traceLevels()->location >= 1)
            {
                locatorInfo->trace(
                    "retrieved adapter for well-known object from locator, adding to locator cache",
                    _reference,
                    r);
            }
            locatorInfo->getEndpoints(r, _reference, _ttl, _callback);
            return;
        }
    }

    if (_reference->getInstance()->traceLevels()->location >= 1)
    {
        locatorInfo->getEndpointsTrace(_reference, endpoints, false);
    }
    if (_callback)
    {
        _callback->setEndpoints(endpoints, false);
    }
}

void
IceInternal::LocatorInfo::RequestCallback::exception(const LocatorInfoPtr& locatorInfo, std::exception_ptr exc)
{
    try
    {
        locatorInfo->getEndpointsException(_reference, exc); // This throws.
    }
    catch (const Ice::LocalException&)
    {
        if (_callback)
        {
            _callback->setException(current_exception());
        }
    }
}

IceInternal::LocatorInfo::RequestCallback::RequestCallback(
    ReferencePtr ref,
    chrono::milliseconds ttl,
    GetEndpointsCallbackPtr cb)
    : _reference(std::move(ref)),
      _ttl(ttl),
      _callback(std::move(cb))
{
}

void
IceInternal::LocatorInfo::Request::addCallback(
    const ReferencePtr& ref,
    const ReferencePtr& wellKnownRef,
    chrono::milliseconds ttl,
    const GetEndpointsCallbackPtr& cb)
{
    RequestCallbackPtr callback = make_shared<RequestCallback>(ref, ttl, cb);
    {
        unique_lock lock(_mutex);
        if (!_response && !_exception)
        {
            _callbacks.push_back(callback);
            if (wellKnownRef) // This request is to resolve the endpoints of a cached well-known object reference
            {
                _wellKnownRefs.push_back(wellKnownRef);
            }
            if (!_sent)
            {
                _sent = true;
                lock.unlock();
                send(); // send() might call exception() from this thread so we need to unlock the mutex.
            }
            return;
        }
    }

    if (_response)
    {
        callback->response(_locatorInfo, _proxy);
    }
    else
    {
        assert(_exception);
        callback->exception(_locatorInfo, _exception);
    }
}

IceInternal::LocatorInfo::GetEndpointsCallback::~GetEndpointsCallback() = default; // avoid weak vtable

IceInternal::LocatorInfo::Request::Request(LocatorInfoPtr locatorInfo, ReferencePtr ref)
    : _locatorInfo(std::move(locatorInfo)),
      _reference(std::move(ref))
{
}

IceInternal::LocatorInfo::Request::~Request() = default; // avoid weak vtable

void
IceInternal::LocatorInfo::Request::response(const optional<ObjectPrx>& proxy)
{
    {
        lock_guard lock(_mutex);
        _locatorInfo->finishRequest(_reference, _wellKnownRefs, proxy, false);
        _response = true;
        _proxy = proxy;
    }
    for (const auto& callback : _callbacks)
    {
        callback->response(_locatorInfo, proxy);
    }
}

void
IceInternal::LocatorInfo::Request::exception(std::exception_ptr ex)
{
    bool isUserException = false;
    try
    {
        rethrow_exception(ex);
    }
    catch (const Ice::UserException&)
    {
        isUserException = true;
    }
    catch (...)
    {
    }

    {
        lock_guard lock(_mutex);
        _locatorInfo->finishRequest(_reference, _wellKnownRefs, nullopt, isUserException);
        _exception = ex;
    }
    for (const auto& callback : _callbacks)
    {
        callback->exception(_locatorInfo, ex);
    }
}

IceInternal::LocatorInfo::LocatorInfo(LocatorPrx locator, LocatorTablePtr table, bool background)
    : _locator(std::move(locator)),
      _table(std::move(table)),
      _background(background)
{
    assert(_table);
}

void
IceInternal::LocatorInfo::destroy()
{
    lock_guard lock(_mutex);
    _locatorRegistry = nullopt;
    _table->clear();
}

bool
IceInternal::LocatorInfo::operator==(const LocatorInfo& rhs) const
{
    return _locator == rhs._locator;
}

bool
IceInternal::LocatorInfo::operator<(const LocatorInfo& rhs) const
{
    return _locator < rhs._locator;
}

optional<LocatorRegistryPrx>
IceInternal::LocatorInfo::getLocatorRegistry()
{
    {
        lock_guard lock(_mutex);
        if (_locatorRegistry)
        {
            // We don't cache the "no locator registry" result.
            return _locatorRegistry;
        }
    }

    //
    // Do not make locator calls from within sync.
    //
    optional<LocatorRegistryPrx> locatorRegistry = _locator->getRegistry();
    if (!locatorRegistry)
    {
        return nullopt;
    }

    {
        lock_guard lock(_mutex);

        //
        // The locator registry can't be located. We use ordered
        // endpoint selection in case the locator returned a proxy
        // with some endpoints which are preferred to be tried first.
        //
        _locatorRegistry =
            locatorRegistry->ice_locator(nullopt)->ice_endpointSelection(Ice::EndpointSelectionType::Ordered);
        return _locatorRegistry;
    }
}

void
IceInternal::LocatorInfo::getEndpoints(
    const ReferencePtr& ref,
    const ReferencePtr& wellKnownRef,
    chrono::milliseconds ttl,
    const GetEndpointsCallbackPtr& callback)
{
    assert(ref->isIndirect());
    vector<EndpointIPtr> endpoints;
    if (!ref->isWellKnown())
    {
        if (!_table->getAdapterEndpoints(ref->getAdapterId(), ttl, endpoints))
        {
            if (_background && !endpoints.empty())
            {
                getAdapterRequest(ref)->addCallback(ref, wellKnownRef, ttl, nullptr);
            }
            else
            {
                getAdapterRequest(ref)->addCallback(ref, wellKnownRef, ttl, callback);
                return;
            }
        }
    }
    else
    {
        ReferencePtr r;
        if (!_table->getObjectReference(ref->getIdentity(), ttl, r))
        {
            if (_background && r)
            {
                getObjectRequest(ref)->addCallback(ref, nullptr, ttl, nullptr);
            }
            else
            {
                getObjectRequest(ref)->addCallback(ref, nullptr, ttl, callback);
                return;
            }
        }

        if (!r->isIndirect())
        {
            endpoints = r->getEndpoints();
        }
        else if (!r->isWellKnown())
        {
            if (ref->getInstance()->traceLevels()->location >= 1)
            {
                trace("found adapter for well-known object in locator cache", ref, r);
            }
            getEndpoints(r, ref, ttl, callback);
            return;
        }
    }

    assert(!endpoints.empty());
    if (ref->getInstance()->traceLevels()->location >= 1)
    {
        getEndpointsTrace(ref, endpoints, true);
    }
    if (callback)
    {
        callback->setEndpoints(endpoints, true);
    }
}

void
IceInternal::LocatorInfo::clearCache(const ReferencePtr& ref)
{
    assert(ref->isIndirect());

    if (!ref->isWellKnown())
    {
        vector<EndpointIPtr> endpoints = _table->removeAdapterEndpoints(ref->getAdapterId());

        if (!endpoints.empty() && ref->getInstance()->traceLevels()->location >= 2)
        {
            trace("removed endpoints for adapter from locator cache", ref, endpoints);
        }
    }
    else
    {
        ReferencePtr r = _table->removeObjectReference(ref->getIdentity());
        if (r)
        {
            if (!r->isIndirect())
            {
                if (ref->getInstance()->traceLevels()->location >= 2)
                {
                    trace("removed endpoints for well-known object from locator cache", ref, r->getEndpoints());
                }
            }
            else if (!r->isWellKnown())
            {
                if (ref->getInstance()->traceLevels()->location >= 2)
                {
                    trace("removed adapter for well-known object from locator cache", ref, r);
                }
                clearCache(r);
            }
        }
    }
}

void
IceInternal::LocatorInfo::getEndpointsException(const ReferencePtr& ref, std::exception_ptr exc)
{
    assert(ref->isIndirect());

    try
    {
        rethrow_exception(exc);
    }
    catch (const AdapterNotFoundException&)
    {
        if (ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
            out << "adapter not found"
                << "\n";
            out << "adapter = " << ref->getAdapterId();
        }

        throw NotRegisteredException(__FILE__, __LINE__, "object adapter", ref->getAdapterId());
    }
    catch (const ObjectNotFoundException&)
    {
        if (ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
            out << "object not found"
                << "\n";
            out << "object = " << Ice::identityToString(ref->getIdentity(), ref->getInstance()->toStringMode());
        }

        throw NotRegisteredException(
            __FILE__,
            __LINE__,
            "object",
            Ice::identityToString(ref->getIdentity(), ref->getInstance()->toStringMode()));
    }
    catch (const NotRegisteredException&)
    {
        throw;
    }
    catch (const LocalException& ex)
    {
        if (ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
            out << "couldn't contact the locator to retrieve endpoints\n";
            if (ref->getAdapterId().empty())
            {
                out << "well-known proxy = " << ref->toString() << "\n";
            }
            else
            {
                out << "adapter = " << ref->getAdapterId() << "\n";
            }
            out << "reason = " << ex;
        }
        throw;
    }
}

void
IceInternal::LocatorInfo::getEndpointsTrace(const ReferencePtr& ref, const vector<EndpointIPtr>& endpoints, bool cached)
{
    if (!endpoints.empty())
    {
        if (cached)
        {
            if (ref->isWellKnown())
            {
                trace("found endpoints for well-known proxy in locator cache", ref, endpoints);
            }
            else
            {
                trace("found endpoints for adapter in locator cache", ref, endpoints);
            }
        }
        else
        {
            if (ref->isWellKnown())
            {
                trace("retrieved endpoints for well-known proxy from locator, adding to locator cache", ref, endpoints);
            }
            else
            {
                trace("retrieved endpoints for adapter from locator, adding to locator cache", ref, endpoints);
            }
        }
    }
    else
    {
        Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
        out << "no endpoints configured for ";
        if (ref->getAdapterId().empty())
        {
            out << "well-known object\n";
            out << "well-known proxy = " << ref->toString();
        }
        else
        {
            out << "adapter\n";
            out << "adapter = " << ref->getAdapterId();
        }
    }
}

void
IceInternal::LocatorInfo::trace(const string& msg, const ReferencePtr& ref, const vector<EndpointIPtr>& endpoints)
{
    assert(ref->isIndirect());

    Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
    out << msg << '\n';
    if (!ref->isWellKnown())
    {
        out << "adapter = " << ref->getAdapterId() << '\n';
    }
    else
    {
        out << "well-known proxy = " << ref->toString() << '\n';
    }

    const char* sep = endpoints.size() > 1 ? ":" : "";
    ostringstream o;
    transform(
        endpoints.begin(),
        endpoints.end(),
        ostream_iterator<string>(o, sep),
        [](const EndpointPtr& endpoint) { return endpoint->toString(); });
    out << "endpoints = " << o.str();
}

void
IceInternal::LocatorInfo::trace(const string& msg, const ReferencePtr& ref, const ReferencePtr& resolved)
{
    assert(ref->isWellKnown());

    Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
    out << msg << '\n';
    out << "well-known proxy = " << ref->toString() << '\n';
    out << "adapter = " << resolved->getAdapterId();
}

IceInternal::LocatorInfo::RequestPtr
IceInternal::LocatorInfo::getAdapterRequest(const ReferencePtr& ref)
{
    lock_guard lock(_mutex);
    if (ref->getInstance()->traceLevels()->location >= 1)
    {
        Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
        out << "searching for adapter by id\nadapter = " << ref->getAdapterId();
    }

    auto p = _adapterRequests.find(ref->getAdapterId());
    if (p != _adapterRequests.end())
    {
        return p->second;
    }

    RequestPtr request = make_shared<AdapterRequest>(shared_from_this(), ref);
    _adapterRequests.insert(make_pair(ref->getAdapterId(), request));
    return request;
}

IceInternal::LocatorInfo::RequestPtr
IceInternal::LocatorInfo::getObjectRequest(const ReferencePtr& ref)
{
    lock_guard lock(_mutex);
    if (ref->getInstance()->traceLevels()->location >= 1)
    {
        Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
        out << "searching for well-known object\nwell-known proxy = " << ref->toString();
    }

    auto p = _objectRequests.find(ref->getIdentity());
    if (p != _objectRequests.end())
    {
        return p->second;
    }
    RequestPtr request = make_shared<ObjectRequest>(shared_from_this(), ref);
    _objectRequests.insert(make_pair(ref->getIdentity(), request));
    return request;
}

void
IceInternal::LocatorInfo::finishRequest(
    const ReferencePtr& ref,
    const vector<ReferencePtr>& wellKnownRefs,
    const std::optional<Ice::ObjectPrx>& proxy,
    bool notRegistered)
{
    if (!proxy || proxy->_getReference()->isIndirect())
    {
        //
        // Remove the cached references of well-known objects for which we tried
        // to resolved the endpoints if these endpoints are empty.
        //
        for (const auto& wellKnownRef : wellKnownRefs)
        {
            _table->removeObjectReference(wellKnownRef->getIdentity());
        }
    }

    if (!ref->isWellKnown())
    {
        if (proxy && !proxy->_getReference()->isIndirect()) // Cache the adapter endpoints.
        {
            _table->addAdapterEndpoints(ref->getAdapterId(), proxy->_getReference()->getEndpoints());
        }
        else if (notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
        {
            _table->removeAdapterEndpoints(ref->getAdapterId());
        }

        lock_guard lock(_mutex);
        assert(_adapterRequests.find(ref->getAdapterId()) != _adapterRequests.end());
        _adapterRequests.erase(ref->getAdapterId());
    }
    else
    {
        if (proxy && !proxy->_getReference()->isWellKnown()) // Cache the well-known object reference.
        {
            _table->addObjectReference(ref->getIdentity(), proxy->_getReference());
        }
        else if (notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
        {
            _table->removeObjectReference(ref->getIdentity());
        }

        lock_guard lock(_mutex);
        assert(_objectRequests.find(ref->getIdentity()) != _objectRequests.end());
        _objectRequests.erase(ref->getIdentity());
    }
}
