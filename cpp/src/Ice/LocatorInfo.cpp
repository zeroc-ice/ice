// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/LocatorInfo.h>
#include <Ice/Locator.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>
#include <Ice/EndpointI.h>
#include <Ice/Reference.h>
#include <Ice/Functional.h>
#include <Ice/Properties.h>
#include <Ice/Comparable.h>
#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(LocatorManager* p) { return p; }
IceUtil::Shared* IceInternal::upCast(LocatorInfo* p) { return p; }
IceUtil::Shared* IceInternal::upCast(LocatorTable* p) { return p; }

namespace
{

class ObjectRequest : public LocatorInfo::Request
{
public:

    ObjectRequest(const LocatorInfoPtr& locatorInfo, const ReferencePtr& ref) : LocatorInfo::Request(locatorInfo, ref)
    {
        assert(ref->isWellKnown());
    }

    virtual void send()
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            LocatorInfo::RequestPtr request = this;
            _locatorInfo->getLocator()->findObjectByIdAsync(
                _reference->getIdentity(),
                [request](const ObjectPrxPtr& object)
                {
                    request->response(object);
                },
                [request](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch(const Exception& ex)
                    {
                        request->exception(ex);
                    }
                });
#else
            _locatorInfo->getLocator()->begin_findObjectById(
                _reference->getIdentity(),
                newCallback_Locator_findObjectById(static_cast<LocatorInfo::Request*>(this),
                                                   &LocatorInfo::Request::response,
                                                   &LocatorInfo::Request::exception));
#endif
        }
        catch(const Ice::Exception& ex)
        {
            exception(ex);
        }
    }
};

class AdapterRequest : public LocatorInfo::Request
{
public:

    AdapterRequest(const LocatorInfoPtr& locatorInfo, const ReferencePtr& ref) : LocatorInfo::Request(locatorInfo, ref)
    {
        assert(ref->isIndirect() && !ref->isWellKnown());
    }

    virtual void send()
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            LocatorInfo::RequestPtr request = this;
            _locatorInfo->getLocator()->findAdapterByIdAsync(_reference->getAdapterId(),
                [request](const shared_ptr<Ice::ObjectPrx>& object)
                {
                    request->response(object);
                },
                [request](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch(const Exception& ex)
                    {
                        request->exception(ex);
                    }
                });
#else
            _locatorInfo->getLocator()->begin_findAdapterById(
                _reference->getAdapterId(),
                newCallback_Locator_findAdapterById(static_cast<LocatorInfo::Request*>(this),
                                                    &LocatorInfo::Request::response,
                                                    &LocatorInfo::Request::exception));
#endif
        }
        catch(const Ice::Exception& ex)
        {
            exception(ex);
        }
    }
};

}

IceInternal::LocatorManager::LocatorManager(const Ice::PropertiesPtr& properties) :
    _background(properties->getPropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0),
    _tableHint(_table.end())
{
}

void
IceInternal::LocatorManager::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

#ifdef ICE_CPP11_MAPPING
    for_each(_table.begin(), _table.end(), [](pair<shared_ptr<Ice::LocatorPrx>, LocatorInfoPtr> it){ it.second->destroy(); });
#else
    for_each(_table.begin(), _table.end(), Ice::secondVoidMemFun<const LocatorPrx, LocatorInfo>(&LocatorInfo::destroy));
#endif
    _table.clear();
    _tableHint = _table.end();

    _locatorTables.clear();
}

LocatorInfoPtr
IceInternal::LocatorManager::get(const LocatorPrxPtr& loc)
{
    if(!loc)
    {
        return 0;
    }

    LocatorPrxPtr locator = loc->ice_locator(0); // The locator can't be located.

    //
    // TODO: reap unused locator info objects?
    //

    IceUtil::Mutex::Lock sync(*this);

    LocatorInfoTable::iterator p = _table.end();

    if(_tableHint != _table.end())
    {
        if(targetEqualTo(_tableHint->first, locator))
        {
            p = _tableHint;
        }
    }

    if(p == _table.end())
    {
        p = _table.find(locator);
    }

    if(p == _table.end())
    {
        //
        // Rely on locator identity for the adapter table. We want to
        // have only one table per locator (not one per locator
        // proxy).
        //
        pair<Identity, EncodingVersion> locatorKey(locator->ice_getIdentity(), locator->ice_getEncodingVersion());
        map<pair<Identity, EncodingVersion>, LocatorTablePtr>::iterator t = _locatorTables.find(locatorKey);
        if(t == _locatorTables.end())
        {
            t = _locatorTables.insert(_locatorTables.begin(),
                                      pair<const pair<Identity, EncodingVersion>, LocatorTablePtr>(
                                          locatorKey, new LocatorTable()));
        }

        _tableHint = _table.insert(_tableHint,
                                   pair<const LocatorPrxPtr, LocatorInfoPtr>(locator,
                                                                          new LocatorInfo(locator, t->second,
                                                                                          _background)));
    }
    else
    {
        _tableHint = p;
    }

    return _tableHint->second;
}

IceInternal::LocatorTable::LocatorTable()
{
}

void
IceInternal::LocatorTable::clear()
{
     IceUtil::Mutex::Lock sync(*this);

     _adapterEndpointsMap.clear();
     _objectMap.clear();
}

bool
IceInternal::LocatorTable::getAdapterEndpoints(const string& adapter, int ttl, vector<EndpointIPtr>& endpoints)
{
    if(ttl == 0) // No locator cache.
    {
        return false;
    }

    IceUtil::Mutex::Lock sync(*this);

    map<string, pair<IceUtil::Time, vector<EndpointIPtr> > >::iterator p = _adapterEndpointsMap.find(adapter);

    if(p != _adapterEndpointsMap.end())
    {
        endpoints = p->second.second;
        return checkTTL(p->second.first, ttl);
    }
    return false;
}

void
IceInternal::LocatorTable::addAdapterEndpoints(const string& adapter, const vector<EndpointIPtr>& endpoints)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, pair<IceUtil::Time, vector<EndpointIPtr> > >::iterator p = _adapterEndpointsMap.find(adapter);

    if(p != _adapterEndpointsMap.end())
    {
        p->second = make_pair(IceUtil::Time::now(IceUtil::Time::Monotonic), endpoints);
    }
    else
    {
        _adapterEndpointsMap.insert(
            make_pair(adapter, make_pair(IceUtil::Time::now(IceUtil::Time::Monotonic), endpoints)));
    }
}

vector<EndpointIPtr>
IceInternal::LocatorTable::removeAdapterEndpoints(const string& adapter)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, pair<IceUtil::Time, vector<EndpointIPtr> > >::iterator p = _adapterEndpointsMap.find(adapter);
    if(p == _adapterEndpointsMap.end())
    {
        return vector<EndpointIPtr>();
    }

    vector<EndpointIPtr> endpoints = p->second.second;

    _adapterEndpointsMap.erase(p);

    return endpoints;
}

bool
IceInternal::LocatorTable::getObjectReference(const Identity& id, int ttl, ReferencePtr& ref)
{
    if(ttl == 0) // No locator cache
    {
        return false;
    }

    IceUtil::Mutex::Lock sync(*this);

    map<Identity, pair<IceUtil::Time, ReferencePtr> >::iterator p = _objectMap.find(id);

    if(p != _objectMap.end())
    {
        ref = p->second.second;
        return checkTTL(p->second.first, ttl);
    }
    return false;
}

void
IceInternal::LocatorTable::addObjectReference(const Identity& id, const ReferencePtr& ref)
{
    IceUtil::Mutex::Lock sync(*this);

    map<Identity, pair<IceUtil::Time, ReferencePtr> >::iterator p = _objectMap.find(id);

    if(p != _objectMap.end())
    {
        p->second = make_pair(IceUtil::Time::now(IceUtil::Time::Monotonic), ref);
    }
    else
    {
        _objectMap.insert(make_pair(id, make_pair(IceUtil::Time::now(IceUtil::Time::Monotonic), ref)));
    }
}

ReferencePtr
IceInternal::LocatorTable::removeObjectReference(const Identity& id)
{
    IceUtil::Mutex::Lock sync(*this);

    map<Identity, pair<IceUtil::Time, ReferencePtr> >::iterator p = _objectMap.find(id);
    if(p == _objectMap.end())
    {
        return 0;
    }

    ReferencePtr ref = p->second.second;
    _objectMap.erase(p);
    return ref;
}

bool
IceInternal::LocatorTable::checkTTL(const IceUtil::Time& time, int ttl) const
{
    assert(ttl != 0);
    if (ttl < 0) // TTL = infinite
    {
        return true;
    }
    else
    {
        return IceUtil::Time::now(IceUtil::Time::Monotonic) - time <= IceUtil::Time::seconds(ttl);
    }
}

void
IceInternal::LocatorInfo::RequestCallback::response(const LocatorInfoPtr& locatorInfo, const Ice::ObjectPrxPtr& proxy)
{
    vector<EndpointIPtr> endpoints;
    if(proxy)
    {
        ReferencePtr r = proxy->_getReference();
        if(_reference->isWellKnown() && !isSupported(_reference->getEncoding(), r->getEncoding()))
        {
            //
            // If a well-known proxy and the returned proxy encoding
            // isn't supported, we're done: there's no compatible
            // endpoint we can use.
            //
        }
        else if(!r->isIndirect())
        {
            endpoints = r->getEndpoints();
        }
        else if(_reference->isWellKnown() && !r->isWellKnown())
        {
            //
            // We're resolving the endpoints of a well-known object and the proxy returned
            // by the locator is an indirect proxy. We now need to resolve the endpoints
            // of this indirect proxy.
            //
            if(_reference->getInstance()->traceLevels()->location >= 1)
            {
                locatorInfo->trace("retrieved adapter for well-known object from locator, adding to locator cache",
                                   _reference, r);
            }
            locatorInfo->getEndpoints(r, _reference, _ttl, _callback);
            return;
        }
    }

    if(_reference->getInstance()->traceLevels()->location >= 1)
    {
        locatorInfo->getEndpointsTrace(_reference, endpoints, false);
    }
    if(_callback)
    {
        _callback->setEndpoints(endpoints, false);
    }
}

void
IceInternal::LocatorInfo::RequestCallback::exception(const LocatorInfoPtr& locatorInfo, const Ice::Exception& exc)
{
    try
    {
        locatorInfo->getEndpointsException(_reference, exc); // This throws.
    }
    catch(const Ice::LocalException& ex)
    {
        if(_callback)
        {
            _callback->setException(ex);
        }
    }
}

IceInternal::LocatorInfo::RequestCallback::RequestCallback(const ReferencePtr& ref,
                                                           int ttl,
                                                           const GetEndpointsCallbackPtr& cb) :
    _reference(ref),  _ttl(ttl), _callback(cb)
{
}

void
IceInternal::LocatorInfo::Request::addCallback(const ReferencePtr& ref,
                                               const ReferencePtr& wellKnownRef,
                                               int ttl,
                                               const GetEndpointsCallbackPtr& cb)
{
    RequestCallbackPtr callback = new RequestCallback(ref, ttl, cb);
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        if(!_response && !_exception)
        {
            _callbacks.push_back(callback);
            if(wellKnownRef) // This request is to resolve the endpoints of a cached well-known object reference
            {
                _wellKnownRefs.push_back(wellKnownRef);
            }
            if(!_sent)
            {
                _sent = true;
                sync.release();
                send(); // send() might call exception() from this thread so we need to release the mutex.
            }
            return;
        }
    }

    if(_response)
    {
        callback->response(_locatorInfo, _proxy);
    }
    else
    {
        assert(_exception);
        callback->exception(_locatorInfo, *_exception);
    }
}

IceInternal::LocatorInfo::Request::Request(const LocatorInfoPtr& locatorInfo, const ReferencePtr& ref) :
    _locatorInfo(locatorInfo), _reference(ref), _sent(false), _response(false)
{
}

void
IceInternal::LocatorInfo::Request::response(const Ice::ObjectPrxPtr& proxy)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _locatorInfo->finishRequest(_reference, _wellKnownRefs, proxy, false);
        _response = true;
        _proxy = proxy;
        _monitor.notifyAll();
    }
    for(vector<RequestCallbackPtr>::const_iterator p = _callbacks.begin(); p != _callbacks.end(); ++p)
    {
        (*p)->response(_locatorInfo, proxy);
    }
}

void
IceInternal::LocatorInfo::Request::exception(const Ice::Exception& ex)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _locatorInfo->finishRequest(_reference, _wellKnownRefs, 0, dynamic_cast<const Ice::UserException*>(&ex));

        ICE_SET_EXCEPTION_FROM_CLONE(_exception, ex.ice_clone());
        _monitor.notifyAll();
    }
    for(vector<RequestCallbackPtr>::const_iterator p = _callbacks.begin(); p != _callbacks.end(); ++p)
    {
        (*p)->exception(_locatorInfo, ex);
    }
}

IceInternal::LocatorInfo::LocatorInfo(const LocatorPrxPtr& locator, const LocatorTablePtr& table, bool background) :
    _locator(locator),
    _table(table),
    _background(background)
{
    assert(_locator);
    assert(_table);
}

void
IceInternal::LocatorInfo::destroy()
{
    IceUtil::Mutex::Lock sync(*this);

    _locatorRegistry = 0;
    _table->clear();
}

bool
IceInternal::LocatorInfo::operator==(const LocatorInfo& rhs) const
{
    return Ice::targetEqualTo(_locator, rhs._locator);
}

bool
IceInternal::LocatorInfo::operator<(const LocatorInfo& rhs) const
{
    return Ice::targetLess(_locator, rhs._locator);
}

LocatorRegistryPrxPtr
IceInternal::LocatorInfo::getLocatorRegistry()
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(_locatorRegistry)
        {
            return _locatorRegistry;
        }
    }

    //
    // Do not make locator calls from within sync.
    //
    LocatorRegistryPrxPtr locatorRegistry = _locator->getRegistry();
    if(!locatorRegistry)
    {
        return 0;
    }

    {
        IceUtil::Mutex::Lock sync(*this);

        //
        // The locator registry can't be located. We use ordered
        // endpoint selection in case the locator returned a proxy
        // with some endpoints which are prefered to be tried first.
        //
        _locatorRegistry = locatorRegistry->ice_locator(0)->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered));
        return _locatorRegistry;
    }
}

void
IceInternal::LocatorInfo::getEndpoints(const ReferencePtr& ref,
                                       const ReferencePtr& wellKnownRef,
                                       int ttl,
                                       const GetEndpointsCallbackPtr& callback)
{
    assert(ref->isIndirect());
    vector<EndpointIPtr> endpoints;
    if(!ref->isWellKnown())
    {
        if(!_table->getAdapterEndpoints(ref->getAdapterId(), ttl, endpoints))
        {
            if(_background && !endpoints.empty())
            {
                getAdapterRequest(ref)->addCallback(ref, wellKnownRef, ttl, 0);
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
        if(!_table->getObjectReference(ref->getIdentity(), ttl, r))
        {
            if(_background && r)
            {
                getObjectRequest(ref)->addCallback(ref, 0, ttl, 0);
            }
            else
            {
                getObjectRequest(ref)->addCallback(ref, 0, ttl, callback);
                return;
            }
        }

        if(!r->isIndirect())
        {
            endpoints = r->getEndpoints();
        }
        else if(!r->isWellKnown())
        {
            if(ref->getInstance()->traceLevels()->location >= 1)
            {
                trace("found adapter for well-known object in locator cache", ref, r);
            }
            getEndpoints(r, ref, ttl, callback);
            return;
        }
    }

    assert(!endpoints.empty());
    if(ref->getInstance()->traceLevels()->location >= 1)
    {
        getEndpointsTrace(ref, endpoints, true);
    }
    if(callback)
    {
        callback->setEndpoints(endpoints, true);
    }
}

void
IceInternal::LocatorInfo::clearCache(const ReferencePtr& ref)
{
    assert(ref->isIndirect());

    if(!ref->isWellKnown())
    {
        vector<EndpointIPtr> endpoints = _table->removeAdapterEndpoints(ref->getAdapterId());

        if(!endpoints.empty() && ref->getInstance()->traceLevels()->location >= 2)
        {
            trace("removed endpoints for adapter from locator cache", ref, endpoints);
        }
    }
    else
    {
        ReferencePtr r = _table->removeObjectReference(ref->getIdentity());
        if(r)
        {
            if(!r->isIndirect())
            {
                if(ref->getInstance()->traceLevels()->location >= 2)
                {
                    trace("removed endpoints for well-known object from locator cache", ref, r->getEndpoints());
                }
            }
            else if(!r->isWellKnown())
            {
                if(ref->getInstance()->traceLevels()->location >= 2)
                {
                    trace("removed adapter for well-known object from locator cache", ref, r);
                }
                clearCache(r);
            }
        }
    }
}

void
IceInternal::LocatorInfo::getEndpointsException(const ReferencePtr& ref, const Ice::Exception& exc)
{
    assert(ref->isIndirect());

    try
    {
        exc.ice_throw();
    }
    catch(const AdapterNotFoundException&)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "adapter not found" << "\n";
            out << "adapter = " << ref->getAdapterId();
        }

        throw NotRegisteredException(__FILE__, __LINE__, "object adapter", ref->getAdapterId());
    }
    catch(const ObjectNotFoundException&)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger,
                      ref->getInstance()->traceLevels()->locationCat);
            out << "object not found" << "\n";
            out << "object = " << Ice::identityToString(ref->getIdentity(),
                                                        ref->getInstance()->toStringMode());
        }

        throw NotRegisteredException(__FILE__, __LINE__, "object",
                                     Ice::identityToString(ref->getIdentity(), ref->getInstance()->toStringMode()));
    }
    catch(const NotRegisteredException&)
    {
        throw;
    }
    catch(const LocalException& ex)
    {
        if(ref->getInstance()->traceLevels()->location >= 1)
        {
            Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
            out << "couldn't contact the locator to retrieve endpoints\n";
            if(ref->getAdapterId().empty())
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
IceInternal::LocatorInfo::getEndpointsTrace(const ReferencePtr& ref,
                                            const vector<EndpointIPtr>& endpoints,
                                            bool cached)
{
    if(!endpoints.empty())
    {
        if(cached)
        {
            if(ref->isWellKnown())
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
            if(ref->isWellKnown())
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
        if(ref->getAdapterId().empty())
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
    if(!ref->isWellKnown())
    {
        out << "adapter = "  << ref->getAdapterId() << '\n';
    }
    else
    {
        out << "well-known proxy = "  << ref->toString() << '\n';
    }

    const char* sep = endpoints.size() > 1 ? ":" : "";
    ostringstream o;
#ifdef ICE_CPP11_MAPPING
    transform(endpoints.begin(), endpoints.end(), ostream_iterator<string>(o, sep),
              [](const EndpointPtr& endpoint)
              {
                  return endpoint->toString();
              });
#else
    transform(endpoints.begin(), endpoints.end(), ostream_iterator<string>(o, sep),
              Ice::constMemFun(&Endpoint::toString));
#endif
    out << "endpoints = " << o.str();
}

void
IceInternal::LocatorInfo::trace(const string& msg, const ReferencePtr& ref, const ReferencePtr& resolved)
{
    assert(ref->isWellKnown());

    Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
    out << msg << '\n';
    out << "well-known proxy = "  << ref->toString() << '\n';
    out << "adapter = " << resolved->getAdapterId();
}

IceInternal::LocatorInfo::RequestPtr
IceInternal::LocatorInfo::getAdapterRequest(const ReferencePtr& ref)
{
    IceUtil::Mutex::Lock sync(*this);
    if(ref->getInstance()->traceLevels()->location >= 1)
    {
        Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
        out << "searching for adapter by id\nadapter = " << ref->getAdapterId();
    }

    map<string, RequestPtr>::const_iterator p = _adapterRequests.find(ref->getAdapterId());
    if(p != _adapterRequests.end())
    {
        return p->second;
    }

    RequestPtr request = new AdapterRequest(this, ref);
    _adapterRequests.insert(make_pair(ref->getAdapterId(), request));
    return request;
}

IceInternal::LocatorInfo::RequestPtr
IceInternal::LocatorInfo::getObjectRequest(const ReferencePtr& ref)
{
    IceUtil::Mutex::Lock sync(*this);
    if(ref->getInstance()->traceLevels()->location >= 1)
    {
        Trace out(ref->getInstance()->initializationData().logger, ref->getInstance()->traceLevels()->locationCat);
        out << "searching for well-known object\nwell-known proxy = " << ref->toString();
    }

    map<Ice::Identity, RequestPtr>::const_iterator p = _objectRequests.find(ref->getIdentity());
    if(p != _objectRequests.end())
    {
        return p->second;
    }
    RequestPtr request = new ObjectRequest(this, ref);
    _objectRequests.insert(make_pair(ref->getIdentity(), request));
    return request;
}

void
IceInternal::LocatorInfo::finishRequest(const ReferencePtr& ref,
                                        const vector<ReferencePtr>& wellKnownRefs,
                                        const Ice::ObjectPrxPtr& proxy,
                                        bool notRegistered)
{
    if(!proxy || proxy->_getReference()->isIndirect())
    {
        //
        // Remove the cached references of well-known objects for which we tried
        // to resolved the endpoints if these endpoints are empty.
        //
        for(vector<ReferencePtr>::const_iterator q = wellKnownRefs.begin(); q != wellKnownRefs.end(); ++q)
        {
            _table->removeObjectReference((*q)->getIdentity());
        }
    }

    if(!ref->isWellKnown())
    {
        if(proxy && !proxy->_getReference()->isIndirect()) // Cache the adapter endpoints.
        {
            _table->addAdapterEndpoints(ref->getAdapterId(), proxy->_getReference()->getEndpoints());
        }
        else if(notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
        {
            _table->removeAdapterEndpoints(ref->getAdapterId());
        }

        IceUtil::Mutex::Lock sync(*this);
        assert(_adapterRequests.find(ref->getAdapterId()) != _adapterRequests.end());
        _adapterRequests.erase(ref->getAdapterId());
    }
    else
    {
        if(proxy && !proxy->_getReference()->isWellKnown()) // Cache the well-known object reference.
        {
            _table->addObjectReference(ref->getIdentity(), proxy->_getReference());
        }
        else if(notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
        {
            _table->removeObjectReference(ref->getIdentity());
        }

        IceUtil::Mutex::Lock sync(*this);
        assert(_objectRequests.find(ref->getIdentity()) != _objectRequests.end());
        _objectRequests.erase(ref->getIdentity());
    }
}
