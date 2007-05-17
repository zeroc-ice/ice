// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <Ice/Direct.h>
#include <Ice/Reference.h>
#include <Ice/EndpointI.h>
#include <Ice/Instance.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/ConnectionI.h> // To convert from ConnectionIPtr to ConnectionPtr in ice_connection().
#include <Ice/Stream.h>
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId, const Context* context)
{
//
// COMPILERBUG: Without this work-around, release VC7.0 and VC7.1
// build crash when FacetNotExistException is raised
//
#if defined(_MSC_VER) && (_MSC_VER >= 1300) && (_MSC_VER <= 1310)
    ObjectPrx fooBar;
#endif

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
IceProxy::Ice::Object::ice_hash() const
{
    return ice_getHash();
}

Int
IceProxy::Ice::Object::ice_getHash() const
{
    return _reference->hash();
}

CommunicatorPtr
IceProxy::Ice::Object::ice_communicator() const
{
    return ice_getCommunicator();
}

CommunicatorPtr
IceProxy::Ice::Object::ice_getCommunicator() const
{
    return _reference->getCommunicator();
}

string
IceProxy::Ice::Object::ice_toString() const
{
    return _reference->toString();
}


bool
IceProxy::Ice::Object::ice_isA(const string& typeId, const Context* context)
{
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __checkTwowayOnly("ice_isA");
            __del = __getDelegate();
            return __del->ice_isA(typeId, context);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, __cnt);
        }
    }
}

void
IceProxy::Ice::Object::ice_ping(const Context* context)
{
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __del = __getDelegate();
            __del->ice_ping(context);
            return;
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, __cnt);
        }
    }
}

vector<string>
IceProxy::Ice::Object::ice_ids(const Context* context)
{
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __checkTwowayOnly("ice_ids");
            __del = __getDelegate();
            return __del->ice_ids(context);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, __cnt);
        }
    }
}

string
IceProxy::Ice::Object::ice_id(const Context* context)
{
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __checkTwowayOnly("ice_id");
            __del = __getDelegate();
            return __del->ice_id(context);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, __cnt);
        }
    }
}


bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const vector<Byte>& inParams,
                                  vector<Byte>& outParams,
                                  const Context* context)
{
    pair<const Byte*, const Byte*> inPair;
    if(inParams.size() == 0)
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &inParams[0];
        inPair.second = inPair.first + inParams.size();
    }
    return ice_invoke(operation, mode, inPair, outParams, context);
}


bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
                                  OperationMode mode,
                                  const pair<const Byte*, const Byte*>& inParams,
                                  vector<Byte>& outParams,
                                  const Context* context)
{
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __del = __getDelegate();
            return __del->ice_invoke(operation, mode, inParams, outParams, context);
        }
        catch(const LocalExceptionWrapper& __ex)
        {
            bool canRetry = mode == Nonmutating || mode == Idempotent;
            if(canRetry)
            {
                __handleExceptionWrapperRelaxed(__del, __ex, __cnt);
            }
            else
            {
                __handleExceptionWrapper(__del, __ex);
            }
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, __cnt);
        }
    }
}

void
IceProxy::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const vector<Byte>& inParams)
{
    cb->__invoke(this, operation, mode, inParams, 0);
}

void
IceProxy::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const vector<Byte>& inParams,
                                        const Context& context)
{
    cb->__invoke(this, operation, mode, inParams, &context);
}

void
IceProxy::Ice::Object::ice_invoke_async(const AMI_Array_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const pair<const Byte*, const Byte*>& inParams)
{
    cb->__invoke(this, operation, mode, inParams, 0);
}

void
IceProxy::Ice::Object::ice_invoke_async(const AMI_Array_Object_ice_invokePtr& cb,
                                        const string& operation,
                                        OperationMode mode,
                                        const pair<const Byte*, const Byte*>& inParams,
                                        const Context& context)
{
    cb->__invoke(this, operation, mode, inParams, &context);
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(_reference->changeIdentity(newIdentity));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_newIdentity(const Identity& newIdentity) const
{
    return ice_identity(newIdentity);
}

Context
IceProxy::Ice::Object::ice_getContext() const
{
    return _reference->getContext()->getValue();
}

ObjectPrx
IceProxy::Ice::Object::ice_context(const Context& newContext) const
{
    ObjectPrx proxy(new ::IceProxy::Ice::Object());
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
}

ObjectPrx
IceProxy::Ice::Object::ice_newContext(const Context& newContext) const
{
    return ice_context(newContext);
}

ObjectPrx
IceProxy::Ice::Object::ice_defaultContext() const
{
    ObjectPrx proxy(new ::IceProxy::Ice::Object());
    proxy->setup(_reference->defaultContext());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(_reference->changeFacet(newFacet));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_newFacet(const string& newFacet) const
{
    return ice_facet(newFacet);
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(_reference->changeAdapterId(newAdapterId));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_newAdapterId(const string& newAdapterId) const
{
    return ice_adapterId(newAdapterId);
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(_reference->changeEndpoints(endpoints));
        return proxy;
    }
}

ObjectPrx
IceProxy::Ice::Object::ice_newEndpoints(const EndpointSeq& newEndpoints) const
{
    return ice_endpoints(newEndpoints);
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(_reference->changeSecure(b));
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(ref);
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isCollocationOptimized() const
{
    return _reference->getCollocationOptimization();
}

ObjectPrx
IceProxy::Ice::Object::ice_collocationOptimization(bool b) const
{
    return ice_collocationOptimized(b);
}

ObjectPrx
IceProxy::Ice::Object::ice_collocationOptimized(bool b) const
{
    if(b == _reference->getCollocationOptimization())
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(_reference->changeCollocationOptimization(b));
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
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
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(ref);
        return proxy;
    }
}

bool
IceProxy::Ice::Object::ice_isThreadPerConnection() const
{
    return _reference->getThreadPerConnection();
}

ObjectPrx
IceProxy::Ice::Object::ice_threadPerConnection(bool b) const
{
    ReferencePtr ref = _reference->changeThreadPerConnection(b);
    if(ref == _reference)
    {
        return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
        ObjectPrx proxy(new ::IceProxy::Ice::Object());
        proxy->setup(ref);
        return proxy;
    }
}

ConnectionPtr
IceProxy::Ice::Object::ice_connection()
{
    return ice_getConnection();
}

ConnectionPtr
IceProxy::Ice::Object::ice_getConnection()
{
    int __cnt = 0;
    while(true)
    {
        Handle< ::IceDelegate::Ice::Object> __del;
        try
        {
            __del = __getDelegate();
            bool compress;
            return __del->__getConnection(compress);
        }
        catch(const LocalException& __ex)
        {
            __handleException(__del, __ex, __cnt);
        }
    }
}

ConnectionPtr
IceProxy::Ice::Object::ice_getCachedConnection() const
{
    Handle< ::IceDelegate::Ice::Object> __del;
    {
        IceUtil::Mutex::Lock sync(*this);
        __del =  _delegate;
    }

    if(__del)
    {
        try
        {
            bool compress;
            return __del->__getConnection(compress);
        }
        catch(const CollocationOptimizationException&)
        {
        }
    }
    return 0;
}

ReferencePtr
IceProxy::Ice::Object::__reference() const
{
    return _reference;
}

void
IceProxy::Ice::Object::__copyFrom(const ObjectPrx& from)
{
    ReferencePtr ref;
    Handle< ::IceDelegateD::Ice::Object> delegateD;
    Handle< ::IceDelegateM::Ice::Object> delegateM;

    {
        IceUtil::Mutex::Lock sync(*from.get());

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

void
IceProxy::Ice::Object::__handleException(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>& delegate,
                                         const LocalException& ex, 
                                         int& cnt)
{
    //
    // Only _delegate needs to be mutex protected here.
    //
    IceUtil::Mutex::Lock sync(*this);
    if(delegate.get() == _delegate.get())
    {
        _delegate = 0;
    }

    ProxyFactoryPtr proxyFactory;
    try
    {
        proxyFactory = _reference->getInstance()->proxyFactory();
    }
    catch(const CommunicatorDestroyedException&)
    {
        //
        // The communicator is already destroyed, so we cannot retry.
        //
        ex.ice_throw();
        
    }

    proxyFactory->checkRetryAfterException(ex, _reference, cnt);
}

void
IceProxy::Ice::Object::__handleExceptionWrapper(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>& delegate,
                                                const LocalExceptionWrapper& ex)
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(delegate.get() == _delegate.get())
        {
            _delegate = 0;
        }
    }

    if(!ex.retry())
    {
        ex.get()->ice_throw();
    }
}

void
IceProxy::Ice::Object::__handleExceptionWrapperRelaxed(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>& del,
                                                       const LocalExceptionWrapper& ex, int& cnt)
{
    if(!ex.retry())
    {
        __handleException(del, *ex.get(), cnt);
    }
    else
    {
        IceUtil::Mutex::Lock sync(*this);
        if(del.get() == _delegate.get())
        {
            _delegate = 0;
        }
    }
}

//
// Overloaded for const char* and const string& because, most of time,
// we call this with a const char* and we want to avoid the overhead
// of constructing a string.
//

void
IceProxy::Ice::Object::__checkTwowayOnly(const char* name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this
    // operation.
    //

    if(!ice_isTwoway())
    {
        TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = name;
        throw ex;
    }
}

void
IceProxy::Ice::Object::__checkTwowayOnly(const string& name) const
{
    //
    // No mutex lock necessary, there is nothing mutable in this
    // operation.
    //

    if(!ice_isTwoway())
    {
        TwowayOnlyException ex(__FILE__, __LINE__);
        ex.operation = name;
        throw ex;
    }
}

ostream&
operator<<(ostream& os, const ::IceProxy::Ice::Object& p)
{
    return os << p.ice_toString();
}

Handle< ::IceDelegate::Ice::Object>
IceProxy::Ice::Object::__getDelegate()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_delegate)
    {
        return _delegate;
    }

    Handle< ::IceDelegate::Ice::Object> delegate;
    if(_reference->getCollocationOptimization())
    {
        ObjectAdapterPtr adapter = _reference->getInstance()->objectAdapterFactory()->findObjectAdapter(this);
        if(adapter)
        {
            Handle< ::IceDelegateD::Ice::Object> d = __createDelegateD();
            d->setup(_reference, adapter);
            delegate = d;
        }
    }

    if(!delegate)
    {
        Handle< ::IceDelegateM::Ice::Object> d = __createDelegateM();
        d->setup(_reference);
        delegate = d;
        
        //
        // If this proxy is for a non-local object, and we are
        // using a router, then add this proxy to the router info
        // object.
        //
        RouterInfoPtr ri = _reference->getRouterInfo();
        if(ri)
        {
            ri->addProxy(this);
        }
    }

    if(_reference->getCacheConnection())
    {
        //
        // The _delegate attribute is only used if "cache connection"
        // is enabled. If it's not enabled, we don't keep track of the
        // delegate -- a new delegate is created for each invocations.
        //
        _delegate = delegate;
    }

    return delegate;
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
IceDelegateM::Ice::Object::ice_isA(const string& __id, const Context* context)
{
    static const string __operation("ice_isA");
    Outgoing __og(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, context, __compress);
    try
    {
        BasicStream* __os = __og.os();
        __os->write(__id, false);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ret;
    bool __ok = __og.invoke();
    try
    {
        BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        __is->read(__ret);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
    return __ret;
}

void
IceDelegateM::Ice::Object::ice_ping(const Context* context)
{
    static const string __operation("ice_ping");
    Outgoing __og(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, context, __compress);
    bool __ok = __og.invoke();
    try
    {
        BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

vector<string>
IceDelegateM::Ice::Object::ice_ids(const Context* context)
{
    static const string __operation("ice_ids");
    Outgoing __og(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, context, __compress);
    vector<string> __ret;
    bool __ok = __og.invoke();
    try
    {
        BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        __is->read(__ret, false);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
    return __ret;
}

string
IceDelegateM::Ice::Object::ice_id(const Context* context)
{
    static const string __operation("ice_id");
    Outgoing __og(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, context, __compress);
    string __ret;
    bool __ok = __og.invoke();
    try
    {
        BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        __is->read(__ret, false);
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
                                      const pair<const Byte*, const Byte*>& inParams,
                                      vector<Byte>& outParams,
                                      const Context* context)
{
    Outgoing __og(__connection.get(), __reference.get(), operation, mode, context, __compress);
    try
    {
        BasicStream* __os = __og.os();
        __os->writeBlob(inParams.first, static_cast<Int>(inParams.second - inParams.first));
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool ok = __og.invoke();
    if(__reference->getMode() == Reference::ModeTwoway)
    {
        try
        {
            BasicStream* __is = __og.is();
            Int sz = __is->getReadEncapsSize();
            __is->readBlob(outParams, sz);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            throw ::IceInternal::LocalExceptionWrapper(__ex, false);
        }
    }
    return ok;
}

ConnectionIPtr
IceDelegateM::Ice::Object::__getConnection(bool& compress) const
{
    compress = __compress;
    return __connection;
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

    assert(!__reference);
    assert(!__connection);

    __reference = from->__reference;
    __connection = from->__connection;
    __compress = from->__compress;
}

void
IceDelegateM::Ice::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!__reference);
    assert(!__connection);

    __reference = ref;
    __connection = __reference->getConnection(__compress);
}

bool
IceDelegateD::Ice::Object::ice_isA(const string& __id, const Context* context)
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
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const LocalException& __ex)
    {
        throw LocalExceptionWrapper(__ex, false);
    }
    return __result;
}

void
IceDelegateD::Ice::Object::ice_ping(const ::Ice::Context* context)
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
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const LocalException& __ex)
    {
        throw LocalExceptionWrapper(__ex, false);
    }
}

vector<string>
IceDelegateD::Ice::Object::ice_ids(const ::Ice::Context* context)
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
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const LocalException& __ex)
    {
        throw LocalExceptionWrapper(__ex, false);
    }
    return __result;
}

string
IceDelegateD::Ice::Object::ice_id(const ::Ice::Context* context)
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
            __direct.servant()->__collocDispatch(__direct);
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
    }
    catch(const LocalException& __ex)
    {
        throw LocalExceptionWrapper(__ex, false);
    }
    return __result;
}

bool
IceDelegateD::Ice::Object::ice_invoke(const string&,
                                      OperationMode,
                                      const pair<const Byte*, const Byte*>& inParams,
                                      vector<Byte>&,
                                      const Context*)
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
    return false;
}

ConnectionIPtr
IceDelegateD::Ice::Object::__getConnection(bool&) const
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
    return 0;
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
        const ImplicitContextIPtr& implicitContext =
                __reference->getInstance()->getImplicitContext();

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
    out->writeProxy(v);
}

void
Ice::ice_readObjectPrx(const InputStreamPtr& in, ObjectPrx& v)
{
    v = in->readProxy();
}
