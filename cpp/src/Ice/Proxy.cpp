// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/Thread.h>

#include <Ice/Proxy.h>
#include <Ice/ProxyFactory.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/ObjectAdapterI.h> // For getIncomingConnections().
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Direct.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/ConnectionFactory.h>
#include <Ice/Connection.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocatorInfo.h>
#include <Ice/BasicStream.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(::IceProxy::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceProxy::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegate::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegate::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegateM::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegateM::Ice::Object* p) { p->__decRef(); }

void IceInternal::incRef(::IceDelegateD::Ice::Object* p) { p->__incRef(); }
void IceInternal::decRef(::IceDelegateD::Ice::Object* p) { p->__decRef(); }

void
IceInternal::checkedCast(const ObjectPrx& b, ObjectPrx& d)
{
    if(b)
    {
        b->__checkTwowayOnly("checkedCast");
    }
    d = b;
}

void
IceInternal::checkedCast(const ObjectPrx& b, const string& f, ObjectPrx& d)
{
    d = 0;
    if(b)
    {
	ObjectPrx bb = b->ice_appendFacet(f);
	try
	{
#ifdef NDEBUG
	    bb->ice_isA("::Ice::Object");
#else
	    bool ok = bb->ice_isA("::Ice::Object");
	    assert(ok);
#endif
	    d = bb;
	}
	catch(const FacetNotExistException&)
	{
	}
    }
}

void
IceInternal::uncheckedCast(const ObjectPrx& b, ObjectPrx& d)
{
    d = b;
}

void
IceInternal::uncheckedCast(const ObjectPrx& b, const string& f, ObjectPrx& d)
{
    d = 0;
    if(b)
    {
	d = b->ice_appendFacet(f);
    }
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
    return _reference->hashValue;
}

bool
IceProxy::Ice::Object::ice_isA(const string& __id)
{
    return ice_isA(__id, _reference->context);
}

bool
IceProxy::Ice::Object::ice_isA(const string& __id, const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_isA");
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->ice_isA(__id, __context);
	}
	catch(const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Ice::Object::ice_ping()
{
    ice_ping(_reference->context);
}

void
IceProxy::Ice::Object::ice_ping(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    __del->ice_ping(__context);
	    return;
	}
	catch(const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

vector<string>
IceProxy::Ice::Object::ice_ids()
{
    return ice_ids(_reference->context);
}

vector<string>
IceProxy::Ice::Object::ice_ids(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_ids");
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->ice_ids(__context);
	}
	catch(const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

string
IceProxy::Ice::Object::ice_id()
{
    return ice_id(_reference->context);
}

string
IceProxy::Ice::Object::ice_id(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_id");
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->ice_id(__context);
	}
	catch(const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

FacetPath
IceProxy::Ice::Object::ice_facets()
{
    return ice_facets(_reference->context);
}

FacetPath
IceProxy::Ice::Object::ice_facets(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_facets");
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->ice_facets(__context);
	}
	catch(const NonRepeatable& __ex)
	{
	    __handleException(*__ex.get(), __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
				  OperationMode mode,
				  const vector<Byte>& inParams,
				  vector<Byte>& outParams)
{
    return ice_invoke(operation, mode, inParams, outParams, _reference->context);
}

bool
IceProxy::Ice::Object::ice_invoke(const string& operation,
				  OperationMode mode,
				  const vector<Byte>& inParams,
				  vector<Byte>& outParams,
				  const Context& context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    return __del->ice_invoke(operation, mode, inParams, outParams, context);
	}
	catch(const NonRepeatable& __ex)
	{
	    bool canRetry = mode == Nonmutating || mode == Idempotent;
	    if(canRetry)
	    {
		__handleException(*__ex.get(), __cnt);
	    }
	    else
	    {
		__rethrowException(*__ex.get());
	    }
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
					const string& operation,
					OperationMode mode,
					const vector<Byte>& inParams)
{
    ice_invoke_async(cb, operation, mode, inParams, _reference->context);
}

void
IceProxy::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
					const string& operation,
					OperationMode mode,
					const vector<Byte>& inParams,
					const Context& context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    Handle< ::IceDelegate::Ice::Object> __del = __getDelegate();
	    __del->ice_invoke_async(cb, operation, mode, inParams, context);
	    return;
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

Context
IceProxy::Ice::Object::ice_getContext() const
{
    return _reference->context;
}

ObjectPrx
IceProxy::Ice::Object::ice_newContext(const Context& newContext) const
{
    if(newContext == _reference->context)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(_reference->changeContext(newContext));
	return proxy;
    }
}

Identity
IceProxy::Ice::Object::ice_getIdentity() const
{
    return _reference->identity;
}

ObjectPrx
IceProxy::Ice::Object::ice_newIdentity(const Identity& newIdentity) const
{
    if(newIdentity == _reference->identity)
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

FacetPath
IceProxy::Ice::Object::ice_getFacet() const
{
    return _reference->facet;
}

ObjectPrx
IceProxy::Ice::Object::ice_newFacet(const FacetPath& newFacet) const
{
    if(newFacet == _reference->facet)
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
IceProxy::Ice::Object::ice_appendFacet(const string& f) const
{
    FacetPath newFacet = _reference->facet;
    newFacet.push_back(f);
    ObjectPrx proxy(new ::IceProxy::Ice::Object());
    proxy->setup(_reference->changeFacet(newFacet));
    return proxy;
}

ObjectPrx
IceProxy::Ice::Object::ice_twoway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeTwoway);
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
IceProxy::Ice::Object::ice_isTwoway() const
{
    return _reference->mode == Reference::ModeTwoway;
}

ObjectPrx
IceProxy::Ice::Object::ice_oneway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeOneway);
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
IceProxy::Ice::Object::ice_isOneway() const
{
    return _reference->mode == Reference::ModeOneway;
}

ObjectPrx
IceProxy::Ice::Object::ice_batchOneway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeBatchOneway);
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
IceProxy::Ice::Object::ice_isBatchOneway() const
{
    return _reference->mode == Reference::ModeBatchOneway;
}

ObjectPrx
IceProxy::Ice::Object::ice_datagram() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeDatagram);
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
IceProxy::Ice::Object::ice_isDatagram() const
{
    return _reference->mode == Reference::ModeDatagram;
}

ObjectPrx
IceProxy::Ice::Object::ice_batchDatagram() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeBatchDatagram);
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
IceProxy::Ice::Object::ice_isBatchDatagram() const
{
    return _reference->mode == Reference::ModeBatchDatagram;
}

ObjectPrx
IceProxy::Ice::Object::ice_secure(bool b) const
{
    ReferencePtr ref = _reference->changeSecure(b);
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

ObjectPrx
IceProxy::Ice::Object::ice_collocationOptimization(bool b) const
{
    ReferencePtr ref = _reference->changeCollocationOptimization(b);
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
IceProxy::Ice::Object::ice_default() const
{
    ReferencePtr ref = _reference->changeDefault();
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

void
IceProxy::Ice::Object::__handleException(const LocalException& ex, int& cnt)
{
    //
    // Only _delegate needs to be mutex protected here.
    //
    {
	IceUtil::Mutex::Lock sync(*this);
	_delegate = 0;
    }

    if(_reference->locatorInfo)
    {
	_reference->locatorInfo->clearObjectCache(_reference);
    }

    ++cnt;
    
    TraceLevelsPtr traceLevels = _reference->instance->traceLevels();
    LoggerPtr logger = _reference->instance->logger();
    ProxyFactoryPtr proxyFactory = _reference->instance->proxyFactory();
    
    //
    // Instance components may be null if Communicator has been destroyed.
    //
    if(traceLevels && logger && proxyFactory)
    {
        const std::vector<int>& retryIntervals = proxyFactory->getRetryIntervals();
        
        if(cnt > static_cast<int>(retryIntervals.size()))
        {
            if(traceLevels->retry >= 1)
            {
                Trace out(logger, traceLevels->retryCat);
                out << "cannot retry operation call because retry limit has been exceeded\n" << ex;
            }
            ex.ice_throw();
        }

        if(traceLevels->retry >= 1)
        {
            Trace out(logger, traceLevels->retryCat);
            out << "re-trying operation call";
            if(cnt > 0 && retryIntervals[cnt - 1] > 0)
            {
                out << " in " << retryIntervals[cnt - 1] << "ms";
            }
            out << " because of exception\n" << ex;
        }

        if(cnt > 0)
        {
            //
            // Sleep before retrying.
            //
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(retryIntervals[cnt - 1]));
        }
    }
    else
    {
        //
        // Impossible to retry after Communicator has been destroyed.
        //
        ex.ice_throw();
    }
}

void
IceProxy::Ice::Object::__rethrowException(const LocalException& ex)
{
    IceUtil::Mutex::Lock sync(*this);

    _delegate = 0;

    if(_reference->locatorInfo)
    {
	_reference->locatorInfo->clearObjectCache(_reference);
    }

    ex.ice_throw();
}

void
IceProxy::Ice::Object::__checkTwowayOnly(const char* name) const
{
    IceUtil::Mutex::Lock sync(*this);

    if(!ice_isTwoway())
    {
        TwowayOnlyException ex(__FILE__, __LINE__);
	ex.operation = name;
	throw ex;
    }
}

Handle< ::IceDelegate::Ice::Object>
IceProxy::Ice::Object::__getDelegate()
{
    IceUtil::Mutex::Lock sync(*this);

    if(!_delegate)
    {
	if(_reference->collocationOptimization)
	{
	    ObjectAdapterPtr adapter = _reference->instance->objectAdapterFactory()->findObjectAdapter(this);
	    if(adapter)
	    {
		Handle< ::IceDelegateD::Ice::Object> delegate = __createDelegateD();
		delegate->setup(_reference, adapter);
		_delegate = delegate;
	    }
	}

	if(!_delegate)
	{
	    Handle< ::IceDelegateM::Ice::Object> delegate = __createDelegateM();
	    delegate->setup(_reference);
	    _delegate = delegate;

	    //
	    // If this proxy is for a non-local object, and we are
	    // using a router, then add this proxy to the router info
	    // object.
	    //
	    if(_reference->routerInfo)
	    {
		_reference->routerInfo->addProxy(this);
	    }
	}
    }

    return _delegate;
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

const Context&
IceProxy::Ice::Object::__defaultContext() const
{
    return _reference->context;
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
    if(__connection)
    {
	__connection->decProxyCount();
    }
}

bool
IceDelegateM::Ice::Object::ice_isA(const string& __id, const Context& __context)
{
    static const string __operation("ice_isA");
    Outgoing __out(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, __context);
    BasicStream* __is = __out.is();
    BasicStream* __os = __out.os();
    __os->write(__id);
    if(!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    bool __ret;
    try
    {
        __is->read(__ret);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::NonRepeatable(__ex);
    }
    return __ret;
}

void
IceDelegateM::Ice::Object::ice_ping(const Context& __context)
{
    static const string __operation("ice_ping");
    Outgoing __out(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, __context);
    if(!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
}

vector<string>
IceDelegateM::Ice::Object::ice_ids(const Context& __context)
{
    static const string __operation("ice_ids");
    Outgoing __out(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, __context);
    BasicStream* __is = __out.is();
    if(!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    vector<string> __ret;
    try
    {
        __is->read(__ret);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::NonRepeatable(__ex);
    }
    return __ret;
}

string
IceDelegateM::Ice::Object::ice_id(const Context& __context)
{
    static const string __operation("ice_id");
    Outgoing __out(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, __context);
    BasicStream* __is = __out.is();
    if(!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    string __ret;
    try
    {
        __is->read(__ret);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::NonRepeatable(__ex);
    }
    return __ret;
}

FacetPath
IceDelegateM::Ice::Object::ice_facets(const Context& __context)
{
    static const string __operation("ice_facets");
    Outgoing __out(__connection.get(), __reference.get(), __operation, ::Ice::Nonmutating, __context);
    BasicStream* __is = __out.is();
    if(!__out.invoke())
    {
	throw ::Ice::UnknownUserException(__FILE__, __LINE__);
    }
    FacetPath __ret;
    try
    {
        __is->read(__ret);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::NonRepeatable(__ex);
    }
    return __ret;
}

bool
IceDelegateM::Ice::Object::ice_invoke(const string& operation,
                                      OperationMode mode,
				      const vector<Byte>& inParams,
				      vector<Byte>& outParams,
				      const Context& context)
{
    Outgoing __out(__connection.get(), __reference.get(), operation, mode, context);
    BasicStream* __os = __out.os();
    __os->writeBlob(inParams);
    bool ok = __out.invoke();
    if(__reference->mode == Reference::ModeTwoway)
    {
        try
        {
            BasicStream* __is = __out.is();
            Int sz = __is->getReadEncapsSize();
            __is->readBlob(outParams, sz);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            throw ::IceInternal::NonRepeatable(__ex);
        }
    }
    return ok;
}

void
IceDelegateM::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr& cb,
					    const string& operation,
					    OperationMode mode,
					    const vector<Byte>& inParams,
					    const Context& context)
{
    cb->__setup(__connection, __reference, operation, mode, context);
    BasicStream* __os = cb->__os();
    __os->writeBlob(inParams);
    cb->__invoke();
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

    if(from->__connection)
    {
	from->__connection->incProxyCount();
    }

// Can not happen, __connection must be null.
/*
    if(__connection)
    {
	__connection->decProxyCount();
    }
*/

    __reference = from->__reference;
    __connection = from->__connection;
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

    if(__reference->reverseAdapter)
    {
	//
	// If we have a reverse object adapter, we use the incoming
	// connections from such object adapter.
	//
	ObjectAdapterIPtr adapter = ObjectAdapterIPtr::dynamicCast(__reference->reverseAdapter);
	assert(adapter);
	list<ConnectionPtr> connections = adapter->getIncomingConnections();

	vector<EndpointPtr> endpoints;
	endpoints.reserve(connections.size());
	transform(connections.begin(), connections.end(), back_inserter(endpoints),
		  ::Ice::constMemFun(&Connection::endpoint));
	endpoints = filterEndpoints(endpoints);
	
	if(endpoints.empty())
	{
	    NoEndpointException ex(__FILE__, __LINE__);
	    ex.proxy = __reference->toString();
	    throw ex;
	}

	list<ConnectionPtr>::iterator p;
	for(p = connections.begin(); p != connections.end(); ++p)
	{
	    if((*p)->endpoint() == endpoints.front())
	    {
		break;
	    }
	}
	assert(p != connections.end());
	__connection = *p;
	__connection->incProxyCount();
    }
    else
    {	
	while(true)
	{
	    bool cached;
	    vector<EndpointPtr> endpoints;

	    if(__reference->routerInfo)
	    {
		//
		// If we route, we send everything to the router's client
		// proxy endpoints.
		//
		ObjectPrx proxy = ref->routerInfo->getClientProxy();
		endpoints = proxy->__reference()->endpoints;
	    }
	    else if(!__reference->endpoints.empty())
	    {
		endpoints = __reference->endpoints;
	    }
	    else if(__reference->locatorInfo)
	    {
		endpoints = __reference->locatorInfo->getEndpoints(__reference, cached);
	    }

	    vector<EndpointPtr> filteredEndpoints = filterEndpoints(endpoints);
	    if(filteredEndpoints.empty())
	    {
		NoEndpointException ex(__FILE__, __LINE__);
		ex.proxy = __reference->toString();
		throw ex;
	    }

	    try
	    {
		OutgoingConnectionFactoryPtr factory = __reference->instance->outgoingConnectionFactory();
		__connection = factory->create(filteredEndpoints);
		assert(__connection);
		__connection->incProxyCount();
	    }
	    catch(const LocalException& ex)
	    {
		if(!__reference->routerInfo && __reference->endpoints.empty())
		{	
		    assert(__reference->locatorInfo);
		    __reference->locatorInfo->clearCache(__reference);
		    
		    if(cached)
		    {
			TraceLevelsPtr traceLevels = __reference->instance->traceLevels();
			LoggerPtr logger = __reference->instance->logger();
			if(traceLevels->retry >= 2)
			{
			    Trace out(logger, traceLevels->retryCat);
			    out << "connection to cached endpoints failed\n"
				<< "removing endpoints from cache and trying one more time\n" << ex;
			}
			continue;
		    }
		}
		
		throw;
	    }

	    break;
	}

	//
	// If we have a router, set the object adapter for this router
	// (if any) to the new connection, so that callbacks from the
	// router can be received over this new connection.
	//
	if(__reference->routerInfo)
	{
	    __connection->setAdapter(__reference->routerInfo->getAdapter());
	}
    }
}

vector<EndpointPtr>
IceDelegateM::Ice::Object::filterEndpoints(const vector<EndpointPtr>& allEndpoints) const
{
    vector<EndpointPtr> endpoints = allEndpoints;

    //
    // Filter out unknown endpoints.
    //
    endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), ::Ice::constMemFun(&Endpoint::unknown)),
                    endpoints.end());

    switch(__reference->mode)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeBatchOneway:
	{
	    //
	    // Filter out datagram endpoints.
	    //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), ::Ice::constMemFun(&Endpoint::datagram)),
                            endpoints.end());
	    break;
	}
	
	case Reference::ModeDatagram:
	case Reference::ModeBatchDatagram:
	{
	    //
	    // Filter out non-datagram endpoints.
	    //
            endpoints.erase(remove_if(endpoints.begin(), endpoints.end(),
                                      not1(::Ice::constMemFun(&Endpoint::datagram))),
                            endpoints.end());
	    break;
	}
    }
    
    //
    // Randomize the order of endpoints.
    //
    random_shuffle(endpoints.begin(), endpoints.end());
    
    //
    // If a secure connection is requested, remove all non-secure
    // endpoints. Otherwise make non-secure endpoints preferred over
    // secure endpoints by partitioning the endpoint vector, so that
    // non-secure endpoints come first.
    //
    if(__reference->secure)
    {
	endpoints.erase(remove_if(endpoints.begin(), endpoints.end(), not1(::Ice::constMemFun(&Endpoint::secure))),
			endpoints.end());
    }
    else
    {
	//
	// We must use stable_partition() instead of just simply
	// partition(), because otherwise some STL implementations
	// order our now randomized endpoints.
	//
	stable_partition(endpoints.begin(), endpoints.end(), not1(::Ice::constMemFun(&Endpoint::secure)));
    }
    
    return endpoints;
}

bool
IceDelegateD::Ice::Object::ice_isA(const string& __id, const Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_isA", ::Ice::Nonmutating, __context);
    while(true)
    {
	Direct __direct(__current);
	return __direct.facetServant()->ice_isA(__id, __current);
    }
    return false; // To keep the Visual C++ compiler happy.
}

void
IceDelegateD::Ice::Object::ice_ping(const ::Ice::Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_ping", ::Ice::Nonmutating, __context);
    while(true)
    {
	Direct __direct(__current);
	__direct.facetServant()->ice_ping(__current);
	return;
    }
}

vector<string>
IceDelegateD::Ice::Object::ice_ids(const ::Ice::Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_ids", ::Ice::Nonmutating, __context);
    while(true)
    {
	Direct __direct(__current);
	return __direct.facetServant()->ice_ids(__current);
    }
    return vector<string>(); // To keep the Visual C++ compiler happy.
}

string
IceDelegateD::Ice::Object::ice_id(const ::Ice::Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_id", ::Ice::Nonmutating, __context);
    while(true)
    {
	Direct __direct(__current);
	return __direct.facetServant()->ice_id(__current);
    }
    return string(); // To keep the Visual C++ compiler happy.
}

FacetPath
IceDelegateD::Ice::Object::ice_facets(const ::Ice::Context& __context)
{
    Current __current;
    __initCurrent(__current, "ice_facets", ::Ice::Nonmutating, __context);
    while(true)
    {
	Direct __direct(__current);
	return __direct.facetServant()->ice_facets(__current);
    }
    return FacetPath(); // To keep the Visual C++ compiler happy.
}

bool
IceDelegateD::Ice::Object::ice_invoke(const string&,
				      OperationMode,
				      const vector<Byte>&,
				      vector<Byte>&,
				      const Context&)
{
    throw CollocationOptimizationException(__FILE__, __LINE__);
}

void
IceDelegateD::Ice::Object::ice_invoke_async(const AMI_Object_ice_invokePtr&,
					    const string&,
					    OperationMode,
					    const vector<Byte>&,
					    const Context&)
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
					 const Context& context)
{
    current.adapter = __adapter;
    current.id = __reference->identity;
    current.facet = __reference->facet;
    current.operation = op;
    current.mode = mode;
    current.ctx = context;
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
	
	FacetPath lhsFacet = lhs->ice_getFacet();
	FacetPath rhsFacet = rhs->ice_getFacet();
	
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
	    FacetPath lhsFacet = lhs->ice_getFacet();
	    FacetPath rhsFacet = rhs->ice_getFacet();
	    
	    if(lhsFacet == rhsFacet)
	    {
		return true;
	    }
	}
	
	return false;
    }
}
