// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <Ice/OutgoingAsync.h>
#include <Ice/Reference.h>
#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/LocalException.h>
#include <Ice/ConnectionI.h> // To convert from ConnectionIPtr to ConnectionPtr in ice_getConnection().
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace Ice
{

const Context noExplicitContext;

}

namespace
{

const string ice_ping_name = "ice_ping";
const string ice_ids_name = "ice_ids";
const string ice_id_name = "ice_id";
const string ice_isA_name = "ice_isA";
const string ice_invoke_name = "ice_invoke";
const string ice_getConnection_name = "ice_getConnection";
const string ice_flushBatchRequests_name = "ice_flushBatchRequests";

}

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace Ice
{

bool
operator<(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return lhs._reference < rhs._reference;
}

bool
operator==(const ObjectPrx& lhs, const ObjectPrx& rhs)
{
    return lhs._reference == rhs._reference;
}

}

void
Ice::ObjectPrx::iceI_ice_isA(const shared_ptr<IceInternal::OutgoingAsyncT<bool>>& outAsync,
                          const string& typeId,
                          const Context& ctx)
{
    iceCheckTwowayOnly(ice_isA_name);
    outAsync->invoke(ice_isA_name, OperationMode::Nonmutating, DefaultFormat, ctx,
                     [&](Ice::OutputStream* os)
                     {
                         os->write(typeId, false);
                     },
                     nullptr);
}

void
Ice::ObjectPrx::iceI_ice_ping(const shared_ptr<IceInternal::OutgoingAsyncT<void>>& outAsync, const Context& ctx)
{
    outAsync->invoke(ice_ping_name, OperationMode::Nonmutating, DefaultFormat, ctx, nullptr, nullptr);
}

void
Ice::ObjectPrx::iceI_ice_ids(const shared_ptr<IceInternal::OutgoingAsyncT<vector<string>>>& outAsync, const Context& ctx)
{
    iceCheckTwowayOnly(ice_ids_name);
    outAsync->invoke(ice_ids_name, OperationMode::Nonmutating, DefaultFormat, ctx, nullptr, nullptr,
                     [](Ice::InputStream* stream)
                     {
                         vector<string> v;
                         stream->read(v, false); // no conversion
                         return v;
                     });
}

void
Ice::ObjectPrx::iceI_ice_id(const shared_ptr<IceInternal::OutgoingAsyncT<string>>& outAsync, const Context& ctx)
{
    iceCheckTwowayOnly(ice_id_name);
    outAsync->invoke(ice_id_name, OperationMode::Nonmutating, DefaultFormat, ctx, nullptr, nullptr,
                     [](Ice::InputStream* stream)
                     {
                         string v;
                         stream->read(v, false); // no conversion
                         return v;
                     });
}

void
Ice::ObjectPrx::iceGetConnection(const shared_ptr<IceInternal::ProxyGetConnection>& outAsync)
{
    outAsync->invoke(ice_getConnection_name);
}

void
Ice::ObjectPrx::iceI_ice_flushBatchRequests(const shared_ptr<IceInternal::ProxyFlushBatchAsync>& outAsync)
{
    outAsync->invoke(ice_flushBatchRequests_name);
}

void
Ice::ObjectPrx::iceCheckTwowayOnly(const string& name) const
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


shared_ptr<ObjectPrx>
Ice::ObjectPrx::iceNewInstance() const
{
    return createProxy<ObjectPrx>();
}

ostream&
Ice::operator<<(ostream& os, const Ice::ObjectPrx& p)
{
    return os << p.ice_toString();
}

#else // C++98 mapping

::Ice::ObjectPrxPtr
IceInternal::checkedCastImpl(const ObjectPrxPtr& b, const string& f, const string& typeId, const Context& context)
{
    if(b != ICE_NULLPTR)
    {
        ObjectPrxPtr bb = b->ice_facet(f);
        try
        {
            if(bb->ice_isA(typeId, context))
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
    return ICE_NULLPTR;
}

bool
IceProxy::Ice::Object::operator==(const Object& r) const
{
    return _reference == r._reference;
}

bool
IceProxy::Ice::Object::operator<(const Object& r) const
{
    return _reference < r._reference;
}

Ice::AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_isA(const string& typeId,
                                       const Context& ctx,
                                       const ::IceInternal::CallbackBasePtr& del,
                                       const ::Ice::LocalObjectPtr& cookie,
                                       bool sync)
{
    iceCheckTwowayOnly(ice_isA_name, sync);
    OutgoingAsyncPtr iceResult = new CallbackOutgoing(this, ice_isA_name, del, cookie, sync);
    try
    {
        iceResult->prepare(ice_isA_name, Nonmutating, ctx);
        ::Ice::OutputStream* iceOs = iceResult->startWriteParams(DefaultFormat);
        iceOs->write(typeId, false);
        iceResult->endWriteParams();
        iceResult->invoke(ice_isA_name);
    }
    catch(const Exception& iceEx)
    {
        iceResult->abort(iceEx);
    }
    return iceResult;
}

bool
IceProxy::Ice::Object::end_ice_isA(const AsyncResultPtr& iceResult)
{
    AsyncResult::iceCheck(iceResult, this, ice_isA_name);
    bool iceOk = iceResult->iceWait();
    if(!iceOk)
    {
        try
        {
            iceResult->iceThrowUserException();
        }
        catch(const UserException& iceEx)
        {
            throw UnknownUserException(__FILE__, __LINE__, iceEx.ice_id());
        }
    }
    bool iceRet;
    ::Ice::InputStream* iceIs = iceResult->iceStartReadParams();
    iceIs->read(iceRet);
    iceResult->iceEndReadParams();
    return iceRet;
}

AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_ping(const Context& ctx,
                                        const ::IceInternal::CallbackBasePtr& del,
                                        const ::Ice::LocalObjectPtr& cookie,
                                        bool sync)
{
    OutgoingAsyncPtr iceResult = new CallbackOutgoing(this, ice_ping_name, del, cookie, sync);
    try
    {
        iceResult->prepare(ice_ping_name, Nonmutating, ctx);
        iceResult->writeEmptyParams();
        iceResult->invoke(ice_ping_name);
    }
    catch(const Exception& iceEx)
    {
        iceResult->abort(iceEx);
    }
    return iceResult;
}

void
IceProxy::Ice::Object::end_ice_ping(const AsyncResultPtr& iceResult)
{
    iceEnd(iceResult, ice_ping_name);
}

AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_ids(const Context& ctx,
                                       const ::IceInternal::CallbackBasePtr& del,
                                       const ::Ice::LocalObjectPtr& cookie,
                                       bool sync)
{
    iceCheckTwowayOnly(ice_ids_name, sync);
    OutgoingAsyncPtr iceResult = new CallbackOutgoing(this, ice_ids_name, del, cookie, sync);
    try
    {
        iceResult->prepare(ice_ids_name, Nonmutating, ctx);
        iceResult->writeEmptyParams();
        iceResult->invoke(ice_ids_name);
    }
    catch(const Exception& iceEx)
    {
        iceResult->abort(iceEx);
    }
    return iceResult;
}

vector<string>
IceProxy::Ice::Object::end_ice_ids(const AsyncResultPtr& iceResult)
{
    AsyncResult::iceCheck(iceResult, this, ice_ids_name);
    bool iceOk = iceResult->iceWait();
    if(!iceOk)
    {
        try
        {
            iceResult->iceThrowUserException();
        }
        catch(const UserException& iceEx)
        {
            throw UnknownUserException(__FILE__, __LINE__, iceEx.ice_id());
        }
    }
    vector<string> iceRet;
    ::Ice::InputStream* iceIs = iceResult->iceStartReadParams();
    iceIs->read(iceRet, false);
    iceResult->iceEndReadParams();
    return iceRet;
}

AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_id(const Context& ctx,
                                      const ::IceInternal::CallbackBasePtr& del,
                                      const ::Ice::LocalObjectPtr& cookie,
                                      bool sync)
{
    iceCheckTwowayOnly(ice_id_name, sync);
    OutgoingAsyncPtr iceResult = new CallbackOutgoing(this, ice_id_name, del, cookie, sync);
    try
    {
        iceResult->prepare(ice_id_name, Nonmutating, ctx);
        iceResult->writeEmptyParams();
        iceResult->invoke(ice_id_name);
    }
    catch(const Exception& iceEx)
    {
        iceResult->abort(iceEx);
    }
    return iceResult;
}

string
IceProxy::Ice::Object::end_ice_id(const AsyncResultPtr& iceResult)
{
    AsyncResult::iceCheck(iceResult, this, ice_id_name);
    bool iceOk = iceResult->iceWait();
    if(!iceOk)
    {
        try
        {
            iceResult->iceThrowUserException();
        }
        catch(const UserException& iceEx)
        {
            throw UnknownUserException(__FILE__, __LINE__, iceEx.ice_id());
        }
    }
    string iceRet;
    ::Ice::InputStream* iceIs = iceResult->iceStartReadParams();
    iceIs->read(iceRet, false);
    iceResult->iceEndReadParams();
    return iceRet;
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const vector<Byte>& inEncaps,
                                  vector<Byte>& outEncaps,
                                  const Context& context)
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

AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_invoke(const string& operation,
                                          OperationMode mode,
                                          const vector<Byte>& inEncaps,
                                          const Context& ctx,
                                          const ::IceInternal::CallbackBasePtr& del,
                                          const ::Ice::LocalObjectPtr& cookie,
                                          bool sync)
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
    return iceI_begin_ice_invoke(operation, mode, inPair, ctx, del, cookie);
}

bool
IceProxy::Ice::Object::end_ice_invoke(vector<Byte>& outEncaps, const AsyncResultPtr& iceResult)
{
    AsyncResult::iceCheck(iceResult, this, ice_invoke_name);
    bool ok = iceResult->iceWait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        const Byte* v;
        Int sz;
        iceResult->iceReadParamEncaps(v, sz);
        vector<Byte>(v, v + sz).swap(outEncaps);
    }
    return ok;
}

AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_invoke(const string& operation,
                                          OperationMode mode,
                                          const pair<const Byte*, const Byte*>& inEncaps,
                                          const Context& ctx,
                                          const ::IceInternal::CallbackBasePtr& del,
                                          const ::Ice::LocalObjectPtr& cookie,
                                          bool sync)
{
    OutgoingAsyncPtr iceResult = new CallbackOutgoing(this, ice_invoke_name, del, cookie, sync);
    try
    {
        iceResult->prepare(operation, mode, ctx);
        iceResult->writeParamEncaps(inEncaps.first, static_cast<Int>(inEncaps.second - inEncaps.first));
        iceResult->invoke(operation);
    }
    catch(const Exception& iceEx)
    {
        iceResult->abort(iceEx);
    }
    return iceResult;
}

bool
IceProxy::Ice::Object::iceI_end_ice_invoke(pair<const Byte*, const Byte*>& outEncaps, const AsyncResultPtr& iceResult)
{
    AsyncResult::iceCheck(iceResult, this, ice_invoke_name);
    bool ok = iceResult->iceWait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        Int sz;
        iceResult->iceReadParamEncaps(outEncaps.first, sz);
        outEncaps.second = outEncaps.first + sz;
    }
    return ok;
}

::Ice::AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_flushBatchRequests(const ::IceInternal::CallbackBasePtr& del,
                                                      const ::Ice::LocalObjectPtr& cookie)
{
    class ProxyFlushBatchAsyncWithCallback : public ProxyFlushBatchAsync, public CallbackCompletion
    {
    public:

        ProxyFlushBatchAsyncWithCallback(const ::Ice::ObjectPrx& proxy,
                                         const CallbackBasePtr& cb,
                                         const ::Ice::LocalObjectPtr& cookie) :
            ProxyFlushBatchAsync(proxy), CallbackCompletion(cb, cookie)
        {
            _cookie = cookie;
        }

        virtual const std::string&
        getOperation() const
        {
            return ice_flushBatchRequests_name;
        }
    };

    ProxyFlushBatchAsyncPtr result = new ProxyFlushBatchAsyncWithCallback(this, del, cookie);
    try
    {
        result->invoke(ice_flushBatchRequests_name);
    }
    catch(const Exception& ex)
    {
        result->abort(ex);
    }
    return result;
}

void
IceProxy::Ice::Object::end_ice_flushBatchRequests(const AsyncResultPtr& result)
{
    AsyncResult::iceCheck(result, this, ice_flushBatchRequests_name);
    result->iceWait();
}

void
IceProxy::Ice::Object::iceEnd(const ::Ice::AsyncResultPtr& iceResult, const std::string& operation) const
{
    AsyncResult::iceCheck(iceResult, this, operation);
    bool iceOk = iceResult->iceWait();
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        if(!iceOk)
        {
            try
            {
                iceResult->iceThrowUserException();
            }
            catch(const UserException& iceEx)
            {
                throw UnknownUserException(__FILE__, __LINE__, iceEx.ice_id());
            }
        }
        iceResult->iceReadEmptyParams();
    }
}

namespace IceProxy
{

namespace Ice
{

ostream&
operator<<(ostream& os, const ::IceProxy::Ice::Object& p)
{
    return os << p.ice_toString();
}

}

}

IceProxy::Ice::Object*
IceProxy::Ice::Object::iceNewInstance() const
{
    return new Object;
}

AsyncResultPtr
IceProxy::Ice::Object::iceI_begin_ice_getConnection(const ::IceInternal::CallbackBasePtr& del,
                                                 const ::Ice::LocalObjectPtr& cookie)
{
    class ProxyGetConnectionWithCallback :  public ProxyGetConnection, public CallbackCompletion
    {
    public:

        ProxyGetConnectionWithCallback(const ::Ice::ObjectPrx& proxy,
                                       const ::IceInternal::CallbackBasePtr& cb,
                                       const ::Ice::LocalObjectPtr& cookie) :
            ProxyGetConnection(proxy), CallbackCompletion(cb, cookie)
        {
            _cookie = cookie;
        }

        virtual const std::string&
        getOperation() const
        {
            return ice_getConnection_name;
        }
    };

    ProxyGetConnectionPtr result = new ProxyGetConnectionWithCallback(this, del, cookie);
    try
    {
        result->invoke(ice_getConnection_name);
    }
    catch(const Exception& ex)
    {
        result->abort(ex);
    }
    return result;
}

ConnectionPtr
IceProxy::Ice::Object::end_ice_getConnection(const AsyncResultPtr& iceResult)
{
    AsyncResult::iceCheck(iceResult, this, ice_getConnection_name);
    iceResult->iceWait();
    return iceResult->getConnection();
}

void
IceProxy::Ice::Object::iceCheckTwowayOnly(const string& name, bool sync) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this operation.
    //
    if(!ice_isTwoway())
    {
        if(sync)
        {
            throw TwowayOnlyException(__FILE__, __LINE__, name);
        }
        else
        {
            throw IceUtil::IllegalArgumentException(__FILE__,
                                                    __LINE__,
                                                    "`" + name + "' can only be called with a twoway proxy");
        }
    }
}

#endif

#ifdef ICE_CPP11_MAPPING
#  define ICE_OBJECT_PRX Ice::ObjectPrx
#  define CONST_POINTER_CAST_OBJECT_PRX const_pointer_cast<ObjectPrx>(shared_from_this())
#else
#  define ICE_OBJECT_PRX IceProxy::Ice::Object
#  define CONST_POINTER_CAST_OBJECT_PRX ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this))
#endif

//
// methods common for both C++11/C++98 mappings
//

Identity
ICE_OBJECT_PRX::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_identity(const Identity& newIdentity) const
{
    if(newIdentity.name.empty())
    {
        throw IllegalIdentityException(__FILE__, __LINE__);
    }
    if(newIdentity == _reference->getIdentity())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
#ifdef ICE_CPP11_MAPPING
        auto proxy = createProxy<ObjectPrx>();
#else
        ObjectPrxPtr proxy = new IceProxy::Ice::Object;
#endif
        proxy->setup(_reference->changeIdentity(newIdentity));
        return proxy;
    }
}

Context
ICE_OBJECT_PRX::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_context(const Context& newContext) const
{
    ObjectPrxPtr proxy = iceNewInstance();
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
}

const string&
ICE_OBJECT_PRX::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_facet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
#ifdef ICE_CPP11_MAPPING
        auto proxy = createProxy<ObjectPrx>();
#else
        ObjectPrx proxy = new IceProxy::Ice::Object;
#endif
        proxy->setup(_reference->changeFacet(newFacet));
        return proxy;
    }
}

string
ICE_OBJECT_PRX::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_adapterId(const string& newAdapterId) const
{
    if(newAdapterId == _reference->getAdapterId())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeAdapterId(newAdapterId));
        return proxy;
    }
}

EndpointSeq
ICE_OBJECT_PRX::ice_getEndpoints() const
{
    vector<EndpointIPtr> endpoints = _reference->getEndpoints();
    EndpointSeq retSeq;
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        retSeq.push_back(ICE_DYNAMIC_CAST(Endpoint, *p));
    }
    return retSeq;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_endpoints(const EndpointSeq& newEndpoints) const
{
    vector<EndpointIPtr> endpoints;
    for(EndpointSeq::const_iterator p = newEndpoints.begin(); p != newEndpoints.end(); ++p)
    {
        endpoints.push_back(ICE_DYNAMIC_CAST(EndpointI, *p));
    }

    if(endpoints == _reference->getEndpoints())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeEndpoints(endpoints));
        return proxy;
    }
}

Int
ICE_OBJECT_PRX::ice_getLocatorCacheTimeout() const
{
    return _reference->getLocatorCacheTimeout();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_locatorCacheTimeout(Int newTimeout) const
{
    if(newTimeout < -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_locatorCacheTimeout: " << newTimeout;
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s.str());
    }
    if(newTimeout == _reference->getLocatorCacheTimeout())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeLocatorCacheTimeout(newTimeout));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isConnectionCached() const
{
    return _reference->getCacheConnection();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_connectionCached(bool newCache) const
{
    if(newCache == _reference->getCacheConnection())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeCacheConnection(newCache));
        return proxy;
    }
}

EndpointSelectionType
ICE_OBJECT_PRX::ice_getEndpointSelection() const
{
    return _reference->getEndpointSelection();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_endpointSelection(EndpointSelectionType newType) const
{
    if(newType == _reference->getEndpointSelection())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeEndpointSelection(newType));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isSecure() const
{
    return _reference->getSecure();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_secure(bool b) const
{
    if(b == _reference->getSecure())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeSecure(b));
        return proxy;
    }
}

::Ice::EncodingVersion
ICE_OBJECT_PRX::ice_getEncodingVersion() const
{
    return _reference->getEncoding();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_encodingVersion(const ::Ice::EncodingVersion& encoding) const
{
    if(encoding == _reference->getEncoding())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeEncoding(encoding));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isPreferSecure() const
{
    return _reference->getPreferSecure();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_preferSecure(bool b) const
{
    if(b == _reference->getPreferSecure())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changePreferSecure(b));
        return proxy;
    }
}

RouterPrxPtr
ICE_OBJECT_PRX::ice_getRouter() const
{
    RouterInfoPtr ri = _reference->getRouterInfo();
#ifdef ICE_CPP11_MAPPING
    return ri ? ri->getRouter() : nullptr;
#else
    return ri ? ri->getRouter() : RouterPrxPtr();
#endif
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_router(const RouterPrxPtr& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(ref);
        return proxy;
    }
}

LocatorPrxPtr
ICE_OBJECT_PRX::ice_getLocator() const
{
    LocatorInfoPtr ri = _reference->getLocatorInfo();
#ifdef ICE_CPP11_MAPPING
    return ri ? ri->getLocator() : nullptr;
#else
    return ri ? ri->getLocator() : LocatorPrxPtr();
#endif
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_locator(const LocatorPrxPtr& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(ref);
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isCollocationOptimized() const
{
    return _reference->getCollocationOptimized();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_collocationOptimized(bool b) const
{
    if(b == _reference->getCollocationOptimized())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeCollocationOptimized(b));
        return proxy;
    }
}

Int
ICE_OBJECT_PRX::ice_getInvocationTimeout() const
{
    return _reference->getInvocationTimeout();
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_invocationTimeout(Int newTimeout) const
{
    if(newTimeout < 1 && newTimeout != -1 && newTimeout != -2)
    {
        ostringstream s;
        s << "invalid value passed to ice_invocationTimeout: " << newTimeout;
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s.str());
    }
    if(newTimeout == _reference->getInvocationTimeout())
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeInvocationTimeout(newTimeout));
        return proxy;
    }
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_twoway() const
{
    if(_reference->getMode() == Reference::ModeTwoway)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeMode(Reference::ModeTwoway));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isTwoway() const
{
    return _reference->getMode() == Reference::ModeTwoway;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_oneway() const
{
    if(_reference->getMode() == Reference::ModeOneway)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeMode(Reference::ModeOneway));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isOneway() const
{
    return _reference->getMode() == Reference::ModeOneway;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_batchOneway() const
{
    if(_reference->getMode() == Reference::ModeBatchOneway)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchOneway));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isBatchOneway() const
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_datagram() const
{
    if(_reference->getMode() == Reference::ModeDatagram)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeMode(Reference::ModeDatagram));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isDatagram() const
{
    return _reference->getMode() == Reference::ModeDatagram;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_batchDatagram() const
{
    if(_reference->getMode() == Reference::ModeBatchDatagram)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(_reference->changeMode(Reference::ModeBatchDatagram));
        return proxy;
    }
}

bool
ICE_OBJECT_PRX::ice_isBatchDatagram() const
{
    return _reference->getMode() == Reference::ModeBatchDatagram;
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_compress(bool b) const
{
    ReferencePtr ref = _reference->changeCompress(b);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_timeout(int t) const
{
    if(t < 1 && t != -1)
    {
        ostringstream s;
        s << "invalid value passed to ice_timeout: " << t;
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, s.str());
    }
    ReferencePtr ref = _reference->changeTimeout(t);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(ref);
        return proxy;
    }
}

ObjectPrxPtr
ICE_OBJECT_PRX::ice_connectionId(const string& id) const
{
    ReferencePtr ref = _reference->changeConnectionId(id);
    if(ref == _reference)
    {
        return CONST_POINTER_CAST_OBJECT_PRX;
    }
    else
    {
        ObjectPrxPtr proxy = iceNewInstance();
        proxy->setup(ref);
        return proxy;
    }
}

string
ICE_OBJECT_PRX::ice_getConnectionId() const
{
    return _reference->getConnectionId();
}

ConnectionPtr
ICE_OBJECT_PRX::ice_getCachedConnection() const
{
    RequestHandlerPtr iceHandler;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        iceHandler =  _requestHandler;
    }

    if(iceHandler)
    {
        try
        {
            return iceHandler->getConnection();
        }
        catch(const LocalException&)
        {
        }
    }
    return 0;
}

void
ICE_OBJECT_PRX::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_requestHandler);

    _reference = ref;
}

int
ICE_OBJECT_PRX::iceHandleException(const Exception& ex,
                                         const RequestHandlerPtr& handler,
                                         OperationMode mode,
                                         bool sent,
                                         int& cnt)
{
    iceUpdateRequestHandler(handler, 0); // Clear the request handler

    //
    // We only retry local exception, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once" (see the implementation of the checkRetryAfterException method
    //  of the ProxyFactory class for the reasons why it can be useful).
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    //
    const LocalException* localEx = dynamic_cast<const LocalException*>(&ex);
    if(localEx && (!sent ||
                   mode == ICE_ENUM(OperationMode, Nonmutating) || mode == ICE_ENUM(OperationMode, Idempotent) ||
                   dynamic_cast<const CloseConnectionException*>(&ex) ||
                   dynamic_cast<const ObjectNotExistException*>(&ex)))
    {
        try
        {
            return _reference->getInstance()->proxyFactory()->checkRetryAfterException(*localEx, _reference, cnt);
        }
        catch(const CommunicatorDestroyedException&)
        {
            //
            // The communicator is already destroyed, so we cannot retry.
            //
            ex.ice_throw();
        }
    }
    else
    {
        ex.ice_throw(); // Retry could break at-most-once semantics, don't retry.
    }
    return 0; // Keep the compiler happy.
}

::IceInternal::RequestHandlerPtr
ICE_OBJECT_PRX::iceGetRequestHandler()
{
    RequestHandlerPtr handler;
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_requestHandler)
        {
            return _requestHandler;
        }
    }
    return _reference->getRequestHandler(ICE_SHARED_FROM_THIS);
}

IceInternal::BatchRequestQueuePtr
ICE_OBJECT_PRX::iceGetBatchRequestQueue()
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_batchRequestQueue)
    {
        _batchRequestQueue = _reference->getBatchRequestQueue();
    }
    return _batchRequestQueue;
}

::IceInternal::RequestHandlerPtr
ICE_OBJECT_PRX::iceSetRequestHandler(const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection())
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(!_requestHandler)
        {
            _requestHandler = handler;
        }
        return _requestHandler;
    }
    return handler;
}

void
ICE_OBJECT_PRX::iceUpdateRequestHandler(const ::IceInternal::RequestHandlerPtr& previous,
                                              const ::IceInternal::RequestHandlerPtr& handler)
{
    if(_reference->getCacheConnection() && previous)
    {
        IceUtil::Mutex::Lock sync(_mutex);
        if(_requestHandler && _requestHandler.get() != handler.get())
        {
            //
            // Update the request handler only if "previous" is the same
            // as the current request handler. This is called after
            // connection binding by the connect request handler. We only
            // replace the request handler if the current handler is the
            // connect request handler.
            //
            _requestHandler = _requestHandler->update(previous, handler);
        }
    }
}

void
ICE_OBJECT_PRX::iceCopyFrom(const ObjectPrxPtr& from)
{
    IceUtil::Mutex::Lock sync(from->_mutex);
    _reference = from->_reference;
    _requestHandler = from->_requestHandler;
}

CommunicatorPtr
ICE_OBJECT_PRX::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
ICE_OBJECT_PRX::ice_toString() const
{
    //
    // Returns the stringified proxy. There's no need to convert the
    // string to a native string: a stringified proxy only contains
    // printable ASCII which is a subset of all native character sets.
    //
    return _reference->toString();
}

Int
ICE_OBJECT_PRX::iceHash() const
{
    return _reference->hash();
}

void
ICE_OBJECT_PRX::iceWrite(OutputStream& os) const
{
    os.write(iceReference()->getIdentity());
    iceReference()->streamWrite(&os);
}

bool
Ice::proxyIdentityLess(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
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
Ice::proxyIdentityEqual(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
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
Ice::proxyIdentityAndFacetLess(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
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
Ice::proxyIdentityAndFacetEqual(const ObjectPrxPtr& lhs, const ObjectPrxPtr& rhs)
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
