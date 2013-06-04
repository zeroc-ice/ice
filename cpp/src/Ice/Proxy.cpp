// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/ConnectRequestHandler.h>
#include <Ice/ConnectionRequestHandler.h>
#include <Ice/Direct.h>
#include <Ice/Reference.h>
#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/ConnectionI.h> // To convert from ConnectionIPtr to ConnectionPtr in ice_getConnection().
#include <Ice/Stream.h>
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

const string ice_ping_name = "ice_ping";
const string ice_ids_name = "ice_ids";
const string ice_id_name = "ice_id";
const string ice_isA_name = "ice_isA";
const string ice_invoke_name = "ice_invoke";
const string ice_flushBatchRequests_name = "ice_flushBatchRequests";

}

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId, const Context* context)
{
    if(b)
    {
        ObjectPrx bb = b->ice_facet(f);
        try
        {
            if(context == 0 ? bb->ice_isA(typeId) : bb->ice_isA(typeId, *context))
            {
                return bb;
            }
#ifndef NDEBUG
            else
            {
                assert(typeId != "::Ice::Object");
            }
#endif
        }
        catch(const FacetNotExistException&)
        {
        }
    }
    return 0;
}

#ifdef ICE_CPP11
void
IceInternal::Cpp11FnOnewayCallbackNC::__completed(const ::Ice::AsyncResultPtr& result) const
{
    try
    {
        result->getProxy()->__end(result, result->getOperation());
    }
    catch(const ::Ice::Exception& ex)
    {
        Cpp11FnCallbackNC::__exception(result, ex);
        return;
    }
    if(_cb != nullptr)
    {
        _cb();
    }
}
#endif

bool
IceProxy::Ice::Object::operator==(const Object& r) const
{
    return _reference == r._reference;
}

bool
IceProxy::Ice::Object::operator!=(const Object& r) const
{
    return _reference != r._reference;
}

bool
IceProxy::Ice::Object::operator<(const Object& r) const
{
    return _reference < r._reference;
}

Int
IceProxy::Ice::Object::ice_getHash() const
{
    return _reference->hash();
}

CommunicatorPtr
IceProxy::Ice::Object::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
IceProxy::Ice::Object::ice_toString() const
{
    //
    // Returns the stringified proxy. There's no need to convert the
    // string to a native string: a stringified proxy only contains
    // printable ASCII which is a subset of all native character sets.
    //
    return _reference->toString();
}


bool
IceProxy::Ice::Object::ice_isA(const string& typeId, const Context* context)
{
    InvocationObserver __observer(this, ice_isA_name, context);
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __checkTwowayOnly(ice_isA_name);
            __del = __getDelegate(false);
            return __del->ice_isA(typeId, context, __observer);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, true, __cnt, __observer);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, true, __cnt, __observer);
        }
    }
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::begin_ice_isA(const string& typeId,
                                     const Context* ctx,
                                     const ::IceInternal::CallbackBasePtr& del,
                                     const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_isA_name, del, cookie);
    __checkAsyncTwowayOnly(ice_isA_name);
    try
    {
        __result->__prepare(ice_isA_name, Nonmutating, ctx);
        IceInternal::BasicStream* __os = __result->__startWriteParams(DefaultFormat);
        __os->write(typeId);
        __result->__endWriteParams();
        __result->__send(true);
    }
    catch(const LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

bool
IceProxy::Ice::Object::end_ice_isA(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_isA_name);
    bool __ok = __result->__wait();
    try
    {
        if(!__ok)
        {
            try
            {
                __result->__throwUserException();
            }
            catch(const UserException& __ex)
            {
                throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        bool __ret;
        IceInternal::BasicStream* __is = __result->__startReadParams();
        __is->read(__ret);
        __result->__endReadParams();
        return __ret;
    }
    catch(const ::Ice::LocalException& ex)
    {
        __result->__getObserver().failed(ex.ice_name());
        throw;
    }
}

void
IceProxy::Ice::Object::ice_ping(const Context* context)
{
    InvocationObserver __observer(this, ice_ping_name, context);
    int __cnt = 0;
    while(true)
    {
        Handle<IceDelegate::Ice::Object> __del;
        try
        {
            __del = __getDelegate(false);
            __del->ice_ping(context, __observer);
            return;
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, true, __cnt, __observer);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, true, __cnt, __observer);
        }
    }
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_ping(const Context* ctx, 
                                      const ::IceInternal::CallbackBasePtr& del,
                                      const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_ping_name, del, cookie);
    try
    {
        __result->__prepare(ice_ping_name, Nonmutating, ctx);
        __result->__writeEmptyParams();
        __result->__send(true);
    }
    catch(const LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

void
IceProxy::Ice::Object::end_ice_ping(const AsyncResultPtr& __result)
{
    __end(__result, ice_ping_name);
}

vector<string>
IceProxy::Ice::Object::ice_ids(const Context* context)
{
    InvocationObserver __observer(this, ice_ids_name, context);
    int __cnt = 0;
    while(true)
    {
        Handle<IceDelegate::Ice::Object> __del;
        try
        {
            __checkTwowayOnly(ice_ids_name);
            __del = __getDelegate(false);
            return __del->ice_ids(context, __observer);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, true, __cnt, __observer);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, true, __cnt, __observer);
        }
    }
}

string
IceProxy::Ice::Object::ice_id(const Context* context)
{
    InvocationObserver __observer(this, ice_id_name, context);
    int __cnt = 0;
    while(true)
    {
        Handle<IceDelegate::Ice::Object> __del;
        try
        {
            __checkTwowayOnly(ice_id_name);
            __del = __getDelegate(false);
            return __del->ice_id(context, __observer);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, true, __cnt, __observer);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, true, __cnt, __observer);
        }
    }
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_ids(const Context* ctx, 
                                     const ::IceInternal::CallbackBasePtr& del,
                                     const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_ids_name, del, cookie);
    __checkAsyncTwowayOnly(ice_ids_name);
    try
    {
        __result->__prepare(ice_ids_name, Nonmutating, ctx);
        __result->__writeEmptyParams();
        __result->__send(true);
    }
    catch(const LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

vector<string>
IceProxy::Ice::Object::end_ice_ids(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_ids_name);
    bool __ok = __result->__wait();
    try
    {
        if(!__ok)
        {
            try
            {
                __result->__throwUserException();
            }
            catch(const UserException& __ex)
            {
                throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        vector<string> __ret;
        IceInternal::BasicStream* __is = __result->__startReadParams();
        __is->read(__ret);
        __result->__endReadParams();
        return __ret;
    }
    catch(const ::Ice::LocalException& ex)
    {
        __result->__getObserver().failed(ex.ice_name());
        throw;
    }
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_id(const Context* ctx, 
                                    const ::IceInternal::CallbackBasePtr& del,
                                    const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_id_name, del, cookie);
    __checkAsyncTwowayOnly(ice_id_name);
    try
    {
        __result->__prepare(ice_id_name, Nonmutating, ctx);
        __result->__writeEmptyParams();
        __result->__send(true);
    }
    catch(const LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

string
IceProxy::Ice::Object::end_ice_id(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_id_name);
    bool __ok = __result->__wait();
    try
    {
        if(!__ok)
        {
            try
            {
                __result->__throwUserException();
            }
            catch(const UserException& __ex)
            {
                throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        string __ret;
        IceInternal::BasicStream* __is = __result->__startReadParams();
        __is->read(__ret);
        __result->__endReadParams();
        return __ret;
    
    }
    catch(const ::Ice::LocalException& ex)
    {
        __result->__getObserver().failed(ex.ice_name());
        throw;
    }
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const vector<Byte>& inEncaps,
                                  vector<Byte>& outEncaps,
                                  const Context* context)
{
    pair<const Byte*, const Byte*> inPair;
    if(inEncaps.empty())
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &inEncaps[0];
        inPair.second = inPair.first + inEncaps.size();
    }
    return ice_invoke(operation, mode, inPair, outEncaps, context);
}


bool
IceProxy::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const vector<Byte>& inEncaps)
{
    Callback_Object_ice_invokePtr del;
    if(dynamic_cast< ::Ice::AMISentCallback*>(cb.get()))
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Object_ice_invoke::__response,
                                            &AMI_Object_ice_invoke::__exception,
                                            &AMI_Object_ice_invoke::__sent);
    }
    else
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Object_ice_invoke::__response,
                                            &AMI_Object_ice_invoke::__exception);
    }
    ::Ice::AsyncResultPtr result = begin_ice_invoke(operation, mode, inEncaps, del);
    return result->sentSynchronously();
}

bool
IceProxy::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const vector<Byte>& inEncaps,
                                        const Context& context)
{
    Callback_Object_ice_invokePtr del;
    if(dynamic_cast< ::Ice::AMISentCallback*>(cb.get()))
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Object_ice_invoke::__response,
                                            &AMI_Object_ice_invoke::__exception,
                                            &AMI_Object_ice_invoke::__sent);
    }
    else
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Object_ice_invoke::__response,
                                            &AMI_Object_ice_invoke::__exception);
    }
    ::Ice::AsyncResultPtr result = begin_ice_invoke(operation, mode, inEncaps, context, del);
    return result->sentSynchronously();
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_invoke(const string& operation,
                                        OperationMode mode,
                                        const vector<Byte>& inEncaps,
                                        const Context* ctx, 
                                        const ::IceInternal::CallbackBasePtr& del,
                                        const ::Ice::LocalObjectPtr& cookie)
{
    pair<const Byte*, const Byte*> inPair;
    if(inEncaps.empty())
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &inEncaps[0];
        inPair.second = inPair.first + inEncaps.size();
    }
    return begin_ice_invoke(operation, mode, inPair, ctx, del, cookie);
}

bool
IceProxy::Ice::Object::end_ice_invoke(vector<Byte>& outEncaps, const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_invoke_name);
    bool ok = __result->__wait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        try
        {
            const Byte* v;
            Int sz;
            __result->__readParamEncaps(v, sz);
            vector<Byte>(v, v + sz).swap(outEncaps);
        }
        catch(const ::Ice::LocalException& ex)
        {
            __result->__getObserver().failed(ex.ice_name());
            throw;
        }
    }
    return ok;
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const pair<const Byte*, const Byte*>& inEncaps,
                                  vector<Byte>& outEncaps,
                                  const Context* context)
{
    InvocationObserver __observer(this, operation, context);
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __del = __getDelegate(false);
            return __del->ice_invoke(operation, mode, inEncaps, outEncaps, context, __observer);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            bool canRetry = mode == Nonmutating || mode == Idempotent;
            if(canRetry)
            {
                __handleExceptionWrapperRelaxed(__del, __ex, true, __cnt, __observer);
            }
            else
            {
                __handleExceptionWrapper(__del, __ex, __observer);
            }
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, true, __cnt, __observer);
        }
    }
}

bool
IceProxy::Ice::Object::ice_invoke_async(const AMI_Array_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const pair<const Byte*, const Byte*>& inEncaps)
{
    Callback_Object_ice_invokePtr del;
    if(dynamic_cast< ::Ice::AMISentCallback*>(cb.get()))
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Array_Object_ice_invoke::__response,
                                            &AMI_Array_Object_ice_invoke::__exception,
                                            &AMI_Array_Object_ice_invoke::__sent);
    }
    else
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Array_Object_ice_invoke::__response,
                                            &AMI_Array_Object_ice_invoke::__exception);
    }
    ::Ice::AsyncResultPtr result = begin_ice_invoke(operation, mode, inEncaps, del);
    return result->sentSynchronously();
}

bool
IceProxy::Ice::Object::ice_invoke_async(const AMI_Array_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const pair<const Byte*, const Byte*>& inEncaps,
                                        const Context& context)
{
    Callback_Object_ice_invokePtr del;
    if(dynamic_cast< ::Ice::AMISentCallback*>(cb.get()))
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Array_Object_ice_invoke::__response,
                                            &AMI_Array_Object_ice_invoke::__exception,
                                            &AMI_Array_Object_ice_invoke::__sent);
    }
    else
    {
        del = newCallback_Object_ice_invoke(cb, 
                                            &AMI_Array_Object_ice_invoke::__response,
                                            &AMI_Array_Object_ice_invoke::__exception);
    }
    ::Ice::AsyncResultPtr result = begin_ice_invoke(operation, mode, inEncaps, context, del);
    return result->sentSynchronously();
}

AsyncResultPtr
IceProxy::Ice::Object::begin_ice_invoke(const string& operation,
                                        OperationMode mode,
                                        const pair<const Byte*, const Byte*>& inEncaps,
                                        const Context* ctx, 
                                        const ::IceInternal::CallbackBasePtr& del,
                                        const ::Ice::LocalObjectPtr& cookie)
{
    OutgoingAsyncPtr __result = new OutgoingAsync(this, ice_invoke_name, del, cookie);
    try
    {
        __result->__prepare(operation, mode, ctx);
        __result->__writeParamEncaps(inEncaps.first, static_cast<Int>(inEncaps.second - inEncaps.first));
        __result->__send(true);
    }
    catch(const LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

bool
IceProxy::Ice::Object::___end_ice_invoke(pair<const Byte*, const Byte*>& outEncaps, const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_invoke_name);
    bool ok = __result->__wait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        try
        {
            Int sz;
            __result->__readParamEncaps(outEncaps.first, sz);
            outEncaps.second = outEncaps.first + sz;
        }
        catch(const ::Ice::LocalException& ex)
        {
            __result->__getObserver().failed(ex.ice_name());
            throw;
        }
    }
    return ok;
}

Identity
IceProxy::Ice::Object::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrx
IceProxy::Ice::Object::ice_identity(const Identity& newIdentity) const
{
    if(newIdentity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__);
    }
    if(newIdentity == _reference->getIdentity())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = new Object;
        proxy->setup(_reference->changeIdentity(newIdentity));
        return proxy;
    }
}

Context
IceProxy::Ice::Object::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

ObjectPrx
IceProxy::Ice::Object::ice_context(const Context& newContext) const
{
    ObjectPrx proxy = __newInstance();
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
}

const string&
IceProxy::Ice::Object::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrx
IceProxy::Ice::Object::ice_facet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = new Object;
        proxy->setup(_reference->changeFacet(newFacet));
        return proxy;
    }
}

string
IceProxy::Ice::Object::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

ObjectPrx
IceProxy::Ice::Object::ice_adapterId(const string& newAdapterId) const
{
    if(newAdapterId == _reference->getAdapterId())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeAdapterId(newAdapterId));
        return proxy;
    }
}

EndpointSeq
IceProxy::Ice::Object::ice_getEndpoints() const
{
    vector<EndpointIPtr> endpoints = _reference->getEndpoints();
    EndpointSeq retSeq;
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        retSeq.push_back(EndpointPtr::dynamicCast(*p));
    }
    return retSeq;
}

ObjectPrx
IceProxy::Ice::Object::ice_endpoints(const EndpointSeq& newEndpoints) const
{
    vector<EndpointIPtr> endpoints;
    for(EndpointSeq::const_iterator p = newEndpoints.begin(); p != newEndpoints.end(); ++p)
    {
        endpoints.push_back(EndpointIPtr::dynamicCast(*p));
    }

    if(endpoints == _reference->getEndpoints())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeEndpoints(endpoints));
        return proxy;
    }
}

Int
IceProxy::Ice::Object::ice_getLocatorCacheTimeout() const
{
    return _reference->getLocatorCacheTimeout();
}

ObjectPrx
IceProxy::Ice::Object::ice_locatorCacheTimeout(Int newTimeout) const
{
    if(newTimeout == _reference->getLocatorCacheTimeout())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeLocatorCacheTimeout(newTimeout));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isConnectionCached() const
{
    return _reference->getCacheConnection();
}

ObjectPrx
IceProxy::Ice::Object::ice_connectionCached(bool newCache) const
{
    if(newCache == _reference->getCacheConnection())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeCacheConnection(newCache));
        return proxy;
    }
}

EndpointSelectionType
IceProxy::Ice::Object::ice_getEndpointSelection() const
{
    return _reference->getEndpointSelection();
}

ObjectPrx
IceProxy::Ice::Object::ice_endpointSelection(EndpointSelectionType newType) const
{
    if(newType == _reference->getEndpointSelection())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeEndpointSelection(newType));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isSecure() const
{
    return _reference->getSecure();
}

ObjectPrx
IceProxy::Ice::Object::ice_secure(bool b) const
{
    if(b == _reference->getSecure())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeSecure(b));
        return proxy;
    }
}

::Ice::EncodingVersion
IceProxy::Ice::Object::ice_getEncodingVersion() const
{
    return _reference->getEncoding();
}

ObjectPrx
IceProxy::Ice::Object::ice_encodingVersion(const ::Ice::EncodingVersion& encoding) const
{
    if(encoding == _reference->getEncoding())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeEncoding(encoding));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isPreferSecure() const
{
    return _reference->getPreferSecure();
}

ObjectPrx
IceProxy::Ice::Object::ice_preferSecure(bool b) const
{
    if(b == _reference->getPreferSecure())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changePreferSecure(b));
        return proxy;
    }
}

RouterPrx
IceProxy::Ice::Object::ice_getRouter() const
{
    RouterInfoPtr ri = _reference->getRouterInfo();
    return ri ? ri->getRouter() : RouterPrx();
}

ObjectPrx
IceProxy::Ice::Object::ice_router(const RouterPrx& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

LocatorPrx
IceProxy::Ice::Object::ice_getLocator() const
{
    LocatorInfoPtr ri = _reference->getLocatorInfo();
    return ri ? ri->getLocator() : LocatorPrx();
}

ObjectPrx
IceProxy::Ice::Object::ice_locator(const LocatorPrx& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isCollocationOptimized() const
{
    return _reference->getCollocationOptimized();
}

ObjectPrx
IceProxy::Ice::Object::ice_collocationOptimized(bool b) const
{
    if(b == _reference->getCollocationOptimized())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeCollocationOptimized(b));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_twoway() const
{
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeTwoway));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isTwoway() const
{
    return _reference->getMode() == Reference::ModeTwoway;
}

ObjectPrx
IceProxy::Ice::Object::ice_oneway() const
{
    if(_reference->getMode() == Reference::ModeOneway)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeOneway));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isOneway() const
{
    return _reference->getMode() == Reference::ModeOneway;
}

ObjectPrx
IceProxy::Ice::Object::ice_batchOneway() const
{
    if(_reference->getMode() == Reference::ModeBatchOneway)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchOneway));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isBatchOneway() const
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}

ObjectPrx
IceProxy::Ice::Object::ice_datagram() const
{
    if(_reference->getMode() == Reference::ModeDatagram)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeDatagram));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isDatagram() const
{
    return _reference->getMode() == Reference::ModeDatagram;
}

ObjectPrx
IceProxy::Ice::Object::ice_batchDatagram() const
{
    if(_reference->getMode() == Reference::ModeBatchDatagram)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchDatagram));
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isBatchDatagram() const
{
    return _reference->getMode() == Reference::ModeBatchDatagram;
}

ObjectPrx
IceProxy::Ice::Object::ice_compress(bool b) const
{
    ReferencePtr ref = _reference->changeCompress(b);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_timeout(int t) const
{
    ReferencePtr ref = _reference->changeTimeout(t);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_connectionId(const string& id) const
{
    ReferencePtr ref = _reference->changeConnectionId(id);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy = __newInstance();
        proxy->setup(ref);
        return proxy;
    }
}

string
IceProxy::Ice::Object::ice_getConnectionId() const
{
    return _reference->getConnectionId();
}

ConnectionPtr
IceProxy::Ice::Object::ice_getConnection()
{
    InvocationObserver __observer(this, "ice_getConnection", 0);
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __del = __getDelegate(false);
            return __del->__getRequestHandler()->getConnection(true); // Wait for the connection to be established.

        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, true, __cnt, __observer);
        }
    }
}

ConnectionPtr
IceProxy::Ice::Object::ice_getCachedConnection() const
{
    Handle< ::IceDelegate::Ice::Object> __del;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        __del =  _delegate;
    }

    if(__del)
    {
        try
        {
            return __del->__getRequestHandler()->getConnection(false);
        }
        catch(const LocalException&)
        {
        }
    }
    return 0;
}

void
IceProxy::Ice::Object::ice_flushBatchRequests()
{
    //
    // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
    // requests were queued with the connection, they would be lost without being noticed.
    //
    InvocationObserver __observer(this, ice_flushBatchRequests_name, 0);
    Handle< ::IceDelegate::Ice::Object> __del;
    int __cnt = -1; // Don't retry.
    try
    {
        __del = __getDelegate(false);
        __del->ice_flushBatchRequests(__observer);
    }
    catch(const LocalException& __ex)
    {
        __handleException(__del, __ex, true, __cnt, __observer);
    }
}

bool
IceProxy::Ice::Object::ice_flushBatchRequests_async(const AMI_Object_ice_flushBatchRequestsPtr& cb)
{
    Callback_Object_ice_flushBatchRequestsPtr __del;
    if(dynamic_cast< AMISentCallback*>(cb.get()))
    {
        __del = newCallback_Object_ice_flushBatchRequests(cb, 
                                                          &AMI_Object_ice_flushBatchRequests::__exception, 
                                                          &AMI_Object_ice_flushBatchRequests::__sent);
    }
    else
    {
        __del = newCallback_Object_ice_flushBatchRequests(cb, &AMI_Object_ice_flushBatchRequests::__exception);
    }
    ::Ice::AsyncResultPtr result = begin_ice_flushBatchRequestsInternal(__del, 0);
    return result->sentSynchronously();
}

::Ice::AsyncResultPtr
IceProxy::Ice::Object::begin_ice_flushBatchRequestsInternal(const ::IceInternal::CallbackBasePtr& del,
                                                            const ::Ice::LocalObjectPtr& cookie)
{
    ::IceInternal::ProxyBatchOutgoingAsyncPtr __result = 
        new ::IceInternal::ProxyBatchOutgoingAsync(this, ice_flushBatchRequests_name, del, cookie);
    try
    {
        __result->__send();
    }
    catch(const LocalException& __ex)
    {
        __result->__exceptionAsync(__ex);
    }
    return __result;
}

void
IceProxy::Ice::Object::end_ice_flushBatchRequests(const AsyncResultPtr& __result)
{
    AsyncResult::__check(__result, this, ice_flushBatchRequests_name);
    __result->__wait();
}

Int
IceProxy::Ice::Object::__hash() const
{
    return _reference->hash();
}

void
IceProxy::Ice::Object::__copyFrom(const ObjectPrx& from)
{
    ReferencePtr ref;
    Handle< ::IceDelegateD::Ice::Object> delegateD;
    Handle< ::IceDelegateM::Ice::Object> delegateM;

    {
        IceUtil::Mutex::Lock sync(from->_mutex);

        ref = from->_reference;
        delegateD = dynamic_cast< ::IceDelegateD::Ice::Object*>(from->_delegate.get());
        delegateM = dynamic_cast< ::IceDelegateM::Ice::Object*>(from->_delegate.get());
    }

    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_delegate);

    _reference = ref;

    if(_reference->getCacheConnection())
    {
        //
        // The _delegate attribute is only used if "cache connection"
        // is enabled. If it's not enabled, we don't keep track of the
        // delegate -- a new delegate is created for each invocations.
        //      

        if(delegateD)
        {
            Handle< ::IceDelegateD::Ice::Object> delegate = __createDelegateD();
            delegate->__copyFrom(delegateD);
            _delegate = delegate;
        }
        else if(delegateM)
        {
            Handle< ::IceDelegateM::Ice::Object> delegate = __createDelegateM();
            delegate->__copyFrom(delegateM);
            _delegate = delegate;
        }
    }
}

int
IceProxy::Ice::Object::__handleException(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>& delegate,
                                         const LocalException& ex, 
                                         bool sleep,
                                         int& cnt,
                                         InvocationObserver& observer)
{
    //
    // Only _delegate needs to be mutex protected here.
    //
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(delegate.get() == _delegate.get())
        {
            _delegate = 0;
        }
    }

    try
    {
        if(cnt == -1) // Don't retry if the retry count is -1.
        {
            ex.ice_throw();
        }

        int interval = 0;
        try
        {
            interval = _reference->getInstance()->proxyFactory()->checkRetryAfterException(ex, _reference, sleep, cnt);
        }
        catch(const CommunicatorDestroyedException&)
        {
            //
            // The communicator is already destroyed, so we cannot retry.
            //
            ex.ice_throw();
        }
        observer.retried();
        return interval;
    }
    catch(const ::Ice::LocalException& ex)
    {
        observer.failed(ex.ice_name());
        throw;
    }
    return 0; // Keep the compiler happy.
}

int
IceProxy::Ice::Object::__handleExceptionWrapper(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>& delegate,
                                                const LocalExceptionWrapper& ex,
                                                InvocationObserver& observer)
{
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(delegate.get() == _delegate.get())
        {
            _delegate = 0;
        }
    }

    if(!ex.retry())
    {
        observer.failed(ex.get()->ice_name());
        ex.get()->ice_throw();
    }

    return 0;
}

int
IceProxy::Ice::Object::__handleExceptionWrapperRelaxed(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>& del,
                                                       const LocalExceptionWrapper& ex, 
                                                       bool sleep,
                                                       int& cnt,
                                                       InvocationObserver& observer)
{
    if(!ex.retry())
    {
        return __handleException(del, *ex.get(), sleep, cnt, observer);
    }
    else
    {
        {
            IceUtil::Mutex::Lock sync(_mutex);
            if(del.get() == _delegate.get())
            {
                _delegate = 0;
            }
        }
        return 0;
    }
}

void
IceProxy::Ice::Object::__checkTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this operation.
    //
    if(!ice_isTwoway())
    {
        TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = name;
        throw ex;
    }
}

void
IceProxy::Ice::Object::__checkAsyncTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this operation.
    //
    if(!ice_isTwoway())
    {
        throw IceUtil::IllegalArgumentException(__FILE__, 
                                                __LINE__, 
                                                "`" + name + "' can only be called with a twoway proxy");
    }
}

void
IceProxy::Ice::Object::__end(const ::Ice::AsyncResultPtr& __result, const std::string& operation) const
{
    AsyncResult::__check(__result, this, operation);
    bool __ok = __result->__wait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        try
        {
            if(!__ok)
            {
                try
                {
                    __result->__throwUserException();
                }
                catch(const UserException& __ex)
                {
                    throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
                }
            }
            __result->__readEmptyParams();
        }
        catch(const ::Ice::LocalException& ex)
        {
            __result->__getObserver().failed(ex.ice_name());
            throw;
        }
    }
}

ostream&
operator<<(ostream& os, const ::IceProxy::Ice::Object& p)
{
    return os << p.ice_toString();
}

Handle< ::IceDelegate::Ice::Object>
IceProxy::Ice::Object::__getDelegate(bool ami)
{
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_delegate)
        {
            return _delegate;
        }
        _delegate = createDelegate(true); // Connect asynchrously to avoid blocking with the proxy mutex locked.
        return _delegate;
    }
    else
    {
        const Reference::Mode mode = _reference->getMode();
        return createDelegate(ami || mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram);
    }
}

void
IceProxy::Ice::Object::__setRequestHandler(const Handle< ::IceDelegate::Ice::Object>& delegate,
                                           const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_delegate.get() == delegate.get())
        {
            if(dynamic_cast< ::IceDelegateM::Ice::Object*>(_delegate.get()))
            {
                _delegate = __createDelegateM();
                _delegate->__setRequestHandler(handler);
            }
            else if(dynamic_cast< ::IceDelegateD::Ice::Object*>(_delegate.get()))
            {
                _delegate = __createDelegateD();
                _delegate->__setRequestHandler(handler);
            }
        }
    }
}

Handle< ::IceDelegateM::Ice::Object>
IceProxy::Ice::Object::__createDelegateM()
{
    return Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Ice::Object);
}

Handle< ::IceDelegateD::Ice::Object>
IceProxy::Ice::Object::__createDelegateD()
{
    return Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Ice::Object);
}

IceProxy::Ice::Object*
IceProxy::Ice::Object::__newInstance() const
{
    return new Object;
}

Handle< ::IceDelegate::Ice::Object>
IceProxy::Ice::Object::createDelegate(bool async)
{
    if(_reference->getCollocationOptimized())
    {
        ObjectAdapterPtr adapter = _reference->getInstance()->objectAdapterFactory()->findObjectAdapter(this);
        if(adapter)
        {
            Handle< ::IceDelegateD::Ice::Object> d = __createDelegateD();
            d->setup(_reference, adapter);
            return d;
        }
    }
    
    Handle< ::IceDelegateM::Ice::Object> d = __createDelegateM();
    d->setup(_reference, this, async);
    return d;
}

void
IceProxy::Ice::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_delegate);

    _reference = ref;
}

IceDelegateM::Ice::Object::~Object()
{
}

bool
IceDelegateM::Ice::Object::ice_isA(const string& __id, const Context* context, InvocationObserver& observer)
{
    Outgoing __og(__handler.get(), ice_isA_name, ::Ice::Nonmutating, context, observer);
    try
    {
        BasicStream* __os = __og.startWriteParams(DefaultFormat);
        __os->write(__id, false);
        __og.endWriteParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ret;
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        BasicStream* __is = __og.startReadParams();
        __is->read(__ret);
        __og.endReadParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
    return __ret;
}

void
IceDelegateM::Ice::Object::ice_ping(const Context* context, InvocationObserver& observer)
{
    Outgoing __og(__handler.get(), ice_ping_name, ::Ice::Nonmutating, context, observer);
    __og.writeEmptyParams();
    bool __ok = __og.invoke();
    if(__og.hasResponse())
    {
        try
        {
            if(!__ok)
            {
                try
                {
                    __og.throwUserException();
                }
                catch(const ::Ice::UserException& __ex)
                {
                    throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
                }
            }
            __og.readEmptyParams();
        }
        catch(const ::Ice::LocalException& __ex)
        {
            throw ::IceInternal::LocalExceptionWrapper(__ex, false);
        }
    }
}

vector<string>
IceDelegateM::Ice::Object::ice_ids(const Context* context, InvocationObserver& observer)
{
    Outgoing __og(__handler.get(), ice_ids_name, ::Ice::Nonmutating, context, observer);
    __og.writeEmptyParams();
    vector<string> __ret;
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        BasicStream* __is = __og.startReadParams();
        __is->read(__ret, false);
        __og.endReadParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
    return __ret;
}

string
IceDelegateM::Ice::Object::ice_id(const Context* context, InvocationObserver& observer)
{
    Outgoing __og(__handler.get(), ice_id_name, ::Ice::Nonmutating, context, observer);
    __og.writeEmptyParams();
    string __ret;
    bool __ok = __og.invoke();
    try
    {
        if(!__ok)
        {
            try
            {
                __og.throwUserException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        BasicStream* __is = __og.startReadParams();
        __is->read(__ret, false);
        __og.endReadParams();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
    return __ret;
}

bool
IceDelegateM::Ice::Object::ice_invoke(const string& operation,
                                      OperationMode mode,
                                      const pair<const Byte*, const Byte*>& inEncaps,
                                      vector<Byte>& outEncaps,
                                      const Context* context,
                                      InvocationObserver& observer)
{
    Outgoing __og(__handler.get(), operation, mode, context, observer);
    try
    {
        __og.writeParamEncaps(inEncaps.first, static_cast<Int>(inEncaps.second - inEncaps.first));
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool ok = __og.invoke();
    if(__handler->getReference()->getMode() == Reference::ModeTwoway)
    {
        try
        {
            const Byte* v;
            Int sz;
            __og.readParamEncaps(v, sz);
            vector<Byte>(v, v + sz).swap(outEncaps);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            throw ::IceInternal::LocalExceptionWrapper(__ex, false);
        }
    }
    return ok;
}

void
IceDelegateM::Ice::Object::ice_flushBatchRequests(InvocationObserver& observer)
{
    BatchOutgoing __og(__handler.get(), observer);
    __og.invoke();
}

RequestHandlerPtr
IceDelegateM::Ice::Object::__getRequestHandler() const
{
    return __handler;
}

void
IceDelegateM::Ice::Object::__setRequestHandler(const RequestHandlerPtr& handler)
{
    __handler = handler;
}

void
IceDelegateM::Ice::Object::__copyFrom(const ::IceInternal::Handle< ::IceDelegateM::Ice::Object>& from)
{
    //
    // No need to synchronize "from", as the delegate is immutable
    // after creation.
    //

    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //
    
    assert(!__handler);
    
    __handler = from->__handler;
}

void
IceDelegateM::Ice::Object::setup(const ReferencePtr& ref, const ::Ice::ObjectPrx& proxy, bool async)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!__handler);

    if(async)
    {
        IceInternal::ConnectRequestHandlerPtr handler = new ::IceInternal::ConnectRequestHandler(ref, proxy, this);
        __handler = handler->connect();
    }
    else
    {
        __handler = new ::IceInternal::ConnectionRequestHandler(ref, proxy);
    }
}

bool
IceDelegateD::Ice::Object::ice_isA(const string& __id, const Context* context, InvocationObserver& /*observer*/)
{ 
    class DirectI : public Direct
    {
    public:

        DirectI(bool& __result, const string& __id, const Current& __current) : 
            Direct(__current),
            _result(__result),
            _id(__id)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            _result = object->ice_isA(_id, _current);
            return DispatchOK;
        }
        
    private:
        
        bool& _result;
        const string& _id;
    };

    Current __current;
    __initCurrent(__current, "ice_isA", ::Ice::Nonmutating, context);
    bool __result;
   
    try
    {
        DirectI __direct(__result, __id, __current);
    
        try
        {
            __direct.getServant()->__collocDispatch(__direct);
        }
        catch(const ::std::exception& __ex)
        {
            __direct.destroy();
            LocalExceptionWrapper::throwWrapper(__ex);
        }
        catch(...)
        {
            __direct.destroy();
            throw UnknownException(__FILE__, __LINE__, "unknown c++ exception");
        }
        __direct.destroy();
    }
    catch(const LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw LocalExceptionWrapper(UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

void
IceDelegateD::Ice::Object::ice_ping(const ::Ice::Context* context, InvocationObserver&)
{
    class DirectI : public Direct
    {
    public:

        DirectI(const Current& __current) : 
            Direct(__current)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            object->ice_ping(_current);
            return DispatchOK;
        }
    };

    Current __current;
    __initCurrent(__current, "ice_ping", ::Ice::Nonmutating, context);
   
    try
    {
        DirectI __direct(__current);
    
        try
        {
            __direct.getServant()->__collocDispatch(__direct);
        }
        catch(const ::std::exception& __ex)
        {
            __direct.destroy();
            LocalExceptionWrapper::throwWrapper(__ex);
        }
        catch(...)
        {
            __direct.destroy();
            throw UnknownException(__FILE__, __LINE__, "unknown c++ exception");
        }
        __direct.destroy();
    }
    catch(const LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw LocalExceptionWrapper(UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
}

vector<string>
IceDelegateD::Ice::Object::ice_ids(const ::Ice::Context* context, InvocationObserver&)
{
    class DirectI : public Direct
    {
    public:

        DirectI(vector<string>& __result, const Current& __current) : 
            Direct(__current),
            _result(__result)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            _result = object->ice_ids(_current);
            return DispatchOK;
        }
        
    private:
        
        vector<string>& _result;
    };

    Current __current;
    __initCurrent(__current, "ice_ids", ::Ice::Nonmutating, context);
    vector<string> __result;
   
    try
    {
        DirectI __direct(__result, __current);
    
        try
        {
            __direct.getServant()->__collocDispatch(__direct);
        }
        catch(const ::std::exception& __ex)
        {
            __direct.destroy();
            LocalExceptionWrapper::throwWrapper(__ex);
        }
        catch(...)
        {
            __direct.destroy();
            throw UnknownException(__FILE__, __LINE__, "unknown c++ exception");
        }
        __direct.destroy();
    }
    catch(const LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw LocalExceptionWrapper(UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

string
IceDelegateD::Ice::Object::ice_id(const ::Ice::Context* context, InvocationObserver&)
{
    class DirectI : public Direct
    {
    public:

        DirectI(string& __result, const Current& __current) : 
            Direct(__current),
            _result(__result)
        {
        }
        
        virtual ::Ice::DispatchStatus
        run(::Ice::Object* object)
        {
            _result = object->ice_id(_current);
            return DispatchOK;
        }
        
    private:
        
        string& _result;
    };

    Current __current;
    __initCurrent(__current, "ice_id", ::Ice::Nonmutating, context);
    string __result;
   
    try
    {
        DirectI __direct(__result, __current);
    
        try
        {
            __direct.getServant()->__collocDispatch(__direct);
        }
        catch(const ::std::exception& __ex)
        {
            __direct.destroy();
            LocalExceptionWrapper::throwWrapper(__ex);
        }
        catch(...)
        {
            __direct.destroy();
            throw UnknownException(__FILE__, __LINE__, "unknown c++ exception");
        }
        __direct.destroy();
    }
    catch(const LocalExceptionWrapper&)
    {
        throw;
    }
    catch(const ::std::exception& __ex)
    {
        LocalExceptionWrapper::throwWrapper(__ex);
    }
    catch(...)
    {
        throw LocalExceptionWrapper(UnknownException(__FILE__, __LINE__, "unknown c++ exception"), false);
    }
    return __result;
}

bool
IceDelegateD::Ice::Object::ice_invoke(const string&,
                                      OperationMode,
                                      const pair<const Byte*, const Byte*>& /*inEncaps*/,
                                      vector<Byte>&,
                                      const Context*,
                                      InvocationObserver&)
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
    return false;
}

void
IceDelegateD::Ice::Object::ice_flushBatchRequests(InvocationObserver& /*observer*/)
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
}

RequestHandlerPtr
IceDelegateD::Ice::Object::__getRequestHandler() const
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
    return 0;
}

void
IceDelegateD::Ice::Object::__setRequestHandler(const RequestHandlerPtr&)
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
}

void
IceDelegateD::Ice::Object::__copyFrom(const ::IceInternal::Handle< ::IceDelegateD::Ice::Object>& from)
{
    //
    // No need to synchronize "from", as the delegate is immutable
    // after creation.
    //

    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!__reference);
    assert(!__adapter);

    __reference = from->__reference;
    __adapter = from->__adapter;
}

void
IceDelegateD::Ice::Object::__initCurrent(Current& current, const string& op, OperationMode mode,
                                         const Context* context)
{
    current.adapter = __adapter;
    current.id = __reference->getIdentity();
    current.facet = __reference->getFacet();
    current.operation = op;
    current.mode = mode;
    if(context != 0)
    {
        //
        // Explicit context
        //
        current.ctx = *context;
    }
    else
    {
        //
        // Implicit context
        //
        const ImplicitContextIPtr& implicitContext = __reference->getInstance()->getImplicitContext();
        const Context& prxContext = __reference->getContext()->getValue();

        if(implicitContext == 0)
        {
            current.ctx = prxContext;
        }
        else
        {
            implicitContext->combine(prxContext, current.ctx);
        }
    }
    current.requestId = -1;
}

void
IceDelegateD::Ice::Object::setup(const ReferencePtr& ref, const ObjectAdapterPtr& adapter)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!__reference);
    assert(!__adapter);

    __reference = ref;
    __adapter = adapter;
}

bool
Ice::proxyIdentityLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() < rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        return lhs->ice_getIdentity() == rhs->ice_getIdentity();
    }
}

bool
Ice::proxyIdentityAndFacetLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return false;
    }
    else if(!lhs && rhs)
    {
        return true;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();
        
        if(lhsIdentity < rhsIdentity)
        {
            return true;
        }
        else if(rhsIdentity < lhsIdentity)
        {
            return false;
        }
        
        string lhsFacet = lhs->ice_getFacet();
        string rhsFacet = rhs->ice_getFacet();
        
        if(lhsFacet < rhsFacet)
        {
            return true;
        }
        else if(rhsFacet < lhsFacet)
        {
            return false;
        }
        
        return false;
    }
}

bool
Ice::proxyIdentityAndFacetEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    if(!lhs && !rhs)
    {
        return true;
    }
    else if(!lhs && rhs)
    {
        return false;
    }
    else if(lhs && !rhs)
    {
        return false;
    }
    else
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();
        
        if(lhsIdentity == rhsIdentity)
        {
            string lhsFacet = lhs->ice_getFacet();
            string rhsFacet = rhs->ice_getFacet();
            
            if(lhsFacet == rhsFacet)
            {
                return true;
            }
        }
        
        return false;
    }
}

void
Ice::ice_writeObjectPrx(const OutputStreamPtr& out, const ObjectPrx& v)
{
    out->write(v);
}

void
Ice::ice_readObjectPrx(const InputStreamPtr& in, ObjectPrx& v)
{
    in->read(v);
}
