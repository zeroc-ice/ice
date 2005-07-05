// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Proxy.h>
#include <IceE/ProxyFactory.h>
#include <IceE/Object.h>
#include <IceE/ObjectAdapterFactory.h>
#include <IceE/Outgoing.h>
#include <IceE/Reference.h>
#include <IceE/Endpoint.h>
#include <IceE/Instance.h>
#include <IceE/RouterInfo.h>
#include <IceE/LocatorInfo.h>
#include <IceE/BasicStream.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(::IceEProxy::IceE::Object* p) { p->__incRef(); }
void IceEInternal::decRef(::IceEProxy::IceE::Object* p) { p->__decRef(); }

void IceEInternal::incRef(::IceEDelegate::IceE::Object* p) { p->__incRef(); }
void IceEInternal::decRef(::IceEDelegate::IceE::Object* p) { p->__decRef(); }

void
IceE::__write(::IceEInternal::BasicStream* __os, const ::IceE::Context& v, ::IceE::__U__Context)
{
    __os->writeSize(::IceE::Int(v.size()));
    ::IceE::Context::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
	__os->write(p->first);
	__os->write(p->second);
    }
}

void
IceE::__read(::IceEInternal::BasicStream* __is, ::IceE::Context& v, ::IceE::__U__Context)
{
    ::IceE::Int sz;
    __is->readSize(sz);
    while(sz--)
    {
	::std::pair<const  ::std::string, ::std::string> pair;
	__is->read(const_cast< ::std::string&>(pair.first));
	::IceE::Context::iterator __i = v.insert(v.end(), pair);
	__is->read(__i->second);
    }
}

::IceE::ObjectPrx
IceEInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId)
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
	ObjectPrx bb = b->ice_newFacet(f);
	try
	{
	    if(bb->ice_isA(typeId))
	    {
		return bb;
	    }
#ifndef NDEBUG
	    else
	    {
		assert(typeId != "::IceE::Object");
	    }
#endif
	}
	catch(const FacetNotExistException&)
	{
	}
    }
    return 0;
}

::IceE::ObjectPrx
IceEInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId, const Context& ctx)
{
//
// COMPILERBUG: Without this work-around, release VC7.0 build crash
// when FacetNotExistException is raised
//
#if defined(_MSC_VER) && (_MSC_VER == 1300)
    ObjectPrx fooBar;
#endif

    if(b)
    {
	ObjectPrx bb = b->ice_newFacet(f);
	try
	{
	    if(bb->ice_isA(typeId, ctx))
	    {
		return bb;
	    }
#ifndef NDEBUG
	    else
	    {
		assert(typeId != "::IceE::Object");
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
IceEProxy::IceE::Object::operator==(const Object& r) const
{
    return _reference == r._reference;
}

bool
IceEProxy::IceE::Object::operator!=(const Object& r) const
{
    return _reference != r._reference;
}

bool
IceEProxy::IceE::Object::operator<(const Object& r) const
{
    return _reference < r._reference;
}

Int
IceEProxy::IceE::Object::ice_hash() const
{
    return _reference->hash();
}

bool
IceEProxy::IceE::Object::ice_isA(const string& __id)
{
    return ice_isA(__id, _reference->getContext());
}

bool
IceEProxy::IceE::Object::ice_isA(const string& __id, const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_isA");
	    Handle< ::IceEDelegate::IceE::Object> __del = __getDelegate();
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
IceEProxy::IceE::Object::ice_ping()
{
    ice_ping(_reference->getContext());
}

void
IceEProxy::IceE::Object::ice_ping(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    Handle< ::IceEDelegate::IceE::Object> __del = __getDelegate();
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
IceEProxy::IceE::Object::ice_ids()
{
    return ice_ids(_reference->getContext());
}

vector<string>
IceEProxy::IceE::Object::ice_ids(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_ids");
	    Handle< ::IceEDelegate::IceE::Object> __del = __getDelegate();
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
IceEProxy::IceE::Object::ice_id()
{
    return ice_id(_reference->getContext());
}

string
IceEProxy::IceE::Object::ice_id(const Context& __context)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly("ice_id");
	    Handle< ::IceEDelegate::IceE::Object> __del = __getDelegate();
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

#ifndef ICEE_PURE_CLIENT
bool
IceEProxy::IceE::Object::ice_invoke(const string& operation,
				  OperationMode mode,
				  const vector<Byte>& inParams,
				  vector<Byte>& outParams)
{
    return ice_invoke(operation, mode, inParams, outParams, _reference->getContext());
}

bool
IceEProxy::IceE::Object::ice_invoke(const string& operation,
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
	    Handle< ::IceEDelegate::IceE::Object> __del = __getDelegate();
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
#endif

Context
IceEProxy::IceE::Object::ice_getContext() const
{
    return _reference->getContext();
}

ObjectPrx
IceEProxy::IceE::Object::ice_newContext(const Context& newContext) const
{
    if(_reference->hasContext() && newContext == _reference->getContext())
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(_reference->changeContext(newContext));
	return proxy;
    }
}

ObjectPrx
IceEProxy::IceE::Object::ice_defaultContext() const
{
    if(!_reference->hasContext())
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(_reference->defaultContext());
	return proxy;
    }
}

Identity
IceEProxy::IceE::Object::ice_getIdentity() const
{
    return _reference->getIdentity();
}

ObjectPrx
IceEProxy::IceE::Object::ice_newIdentity(const Identity& newIdentity) const
{
    if(newIdentity == _reference->getIdentity())
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(_reference->changeIdentity(newIdentity));
	return proxy;
    }
}

const string&
IceEProxy::IceE::Object::ice_getFacet() const
{
    return _reference->getFacet();
}

ObjectPrx
IceEProxy::IceE::Object::ice_newFacet(const string& newFacet) const
{
    if(newFacet == _reference->getFacet())
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(_reference->changeFacet(newFacet));
	return proxy;
    }
}

ObjectPrx
IceEProxy::IceE::Object::ice_twoway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeTwoway);
    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

bool
IceEProxy::IceE::Object::ice_isTwoway() const
{
    return _reference->getMode() == Reference::ModeTwoway;
}

ObjectPrx
IceEProxy::IceE::Object::ice_oneway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeOneway);
    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

bool
IceEProxy::IceE::Object::ice_isOneway() const
{
    return _reference->getMode() == Reference::ModeOneway;
}

#ifndef ICEE_NO_BATCH
ObjectPrx
IceEProxy::IceE::Object::ice_batchOneway() const
{
    ReferencePtr ref = _reference->changeMode(Reference::ModeBatchOneway);
    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

bool
IceEProxy::IceE::Object::ice_isBatchOneway() const
{
    return _reference->getMode() == Reference::ModeBatchOneway;
}
#endif

ObjectPrx
IceEProxy::IceE::Object::ice_timeout(int t) const
{
    ReferencePtr ref = _reference->changeTimeout(t);
    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

#ifndef ICEE_NO_ROUTER

ObjectPrx
IceEProxy::IceE::Object::ice_router(const RouterPrx& router) const
{
    ReferencePtr ref = _reference->changeRouter(router);
    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

#endif

#ifndef ICEE_NO_LOCATOR

ObjectPrx
IceEProxy::IceE::Object::ice_locator(const LocatorPrx& locator) const
{
    ReferencePtr ref = _reference->changeLocator(locator);
    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

#endif

ObjectPrx
IceEProxy::IceE::Object::ice_default() const
{
    ReferencePtr ref = _reference->changeDefault();
    ref = ref->changeDefault();

    if(ref == _reference)
    {
	return ObjectPrx(const_cast< ::IceEProxy::IceE::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceEProxy::IceE::Object());
	proxy->setup(ref);
	return proxy;
    }
}

ConnectionPtr
IceEProxy::IceE::Object::ice_connection()
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    Handle< ::IceEDelegate::IceE::Object> __del = __getDelegate();
	    return __del->ice_connection();
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

ReferencePtr
IceEProxy::IceE::Object::__reference() const
{
    return _reference;
}

void
IceEProxy::IceE::Object::__copyFrom(const ObjectPrx& from)
{
    ReferencePtr ref;
    Handle< ::IceEDelegate::IceE::Object> delegate;

    {
	::IceE::Mutex::Lock sync(*from.get());

	ref = from->_reference;
	delegate = dynamic_cast< ::IceEDelegate::IceE::Object*>(from->_delegate.get());
    }

    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_delegate);

    _reference = ref;

    if(delegate)
    {
	_delegate = __createDelegate();
	_delegate->__copyFrom(delegate);
    }
}

void
IceEProxy::IceE::Object::__handleException(const LocalException& ex, int& cnt)
{
    //
    // Only _delegate needs to be mutex protected here.
    //
    {
	::IceE::Mutex::Lock sync(*this);
	_delegate = 0;
    }

    ProxyFactoryPtr proxyFactory = _reference->getInstance()->proxyFactory();
    if(proxyFactory)
    {
	proxyFactory->checkRetryAfterException(ex, _reference, cnt);
    }
    else
    {
	//
	// The communicator is already destroyed, so we cannot retry.
	//
        ex.ice_throw();
    }
}

void
IceEProxy::IceE::Object::__rethrowException(const LocalException& ex)
{
    //
    // Only _delegate needs to be mutex protected here.
    //
    {
	::IceE::Mutex::Lock sync(*this);
	_delegate = 0;
    }

    ex.ice_throw();
}

void
IceEProxy::IceE::Object::__checkTwowayOnly(const char* name) const
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

Handle< ::IceEDelegate::IceE::Object>
IceEProxy::IceE::Object::__getDelegate()
{
    ::IceE::Mutex::Lock sync(*this);

    if(!_delegate)
    {
	Handle< ::IceEDelegate::IceE::Object> delegate = __createDelegate();
	delegate->setup(_reference);
	_delegate = delegate;

	//
	// If this proxy is for a non-local object, and we are
	// using a router, then add this proxy to the router info
	// object.
	//
#ifndef ICEE_NO_ROUTER
	RoutableReferencePtr rr = RoutableReferencePtr::dynamicCast(_reference);
	if(rr && rr->getRouterInfo())
	{
	    rr->getRouterInfo()->addProxy(this);
	}
#endif
    }

    return _delegate;
}

Handle< ::IceEDelegate::IceE::Object>
IceEProxy::IceE::Object::__createDelegate()
{
    return Handle< ::IceEDelegate::IceE::Object>(new ::IceEDelegate::IceE::Object);
}

const Context&
IceEProxy::IceE::Object::__defaultContext() const
{
    return _reference->getContext();
}

void
IceEProxy::IceE::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_delegate);

    _reference = ref;
}

IceEDelegate::IceE::Object::~Object()
{
}

bool
IceEDelegate::IceE::Object::ice_isA(const string& __id, const Context& __context)
{
    static const string __operation("ice_isA");
    Outgoing __outS(__connection.get(), __reference.get(), __operation, ::IceE::Nonmutating, __context);
    BasicStream* __is = __outS.is();
    BasicStream* __os = __outS.os();
    __os->write(__id);
    bool __ret;
    try
    {
	if(!__outS.invoke())
	{
	    __is->throwException();
	}
        __is->read(__ret);
    }
    catch(const ::IceE::LocalException& __ex)
    {
        throw ::IceEInternal::NonRepeatable(__ex);
    }
    return __ret;
}

void
IceEDelegate::IceE::Object::ice_ping(const Context& __context)
{
    static const string __operation("ice_ping");
    Outgoing __outS(__connection.get(), __reference.get(), __operation, ::IceE::Nonmutating, __context);
    BasicStream* __is = __outS.is();
    try
    {
	if(!__outS.invoke())
	{
	    __is->throwException();
	}
    }
    catch(const ::IceE::LocalException& __ex)
    {
        throw ::IceEInternal::NonRepeatable(__ex);
    }
}

vector<string>
IceEDelegate::IceE::Object::ice_ids(const Context& __context)
{
    static const string __operation("ice_ids");
    Outgoing __outS(__connection.get(), __reference.get(), __operation, ::IceE::Nonmutating, __context);
    BasicStream* __is = __outS.is();
    vector<string> __ret;
    try
    {
	if(!__outS.invoke())
	{
	    __is->throwException();
	}
        __is->read(__ret);
    }
    catch(const ::IceE::LocalException& __ex)
    {
        throw ::IceEInternal::NonRepeatable(__ex);
    }
    return __ret;
}

string
IceEDelegate::IceE::Object::ice_id(const Context& __context)
{
    static const string __operation("ice_id");
    Outgoing __outS(__connection.get(), __reference.get(), __operation, ::IceE::Nonmutating, __context);
    BasicStream* __is = __outS.is();
    string __ret;
    try
    {
	if(!__outS.invoke())
	{
	    __is->throwException();
	}
        __is->read(__ret);
    }
    catch(const ::IceE::LocalException& __ex)
    {
        throw ::IceEInternal::NonRepeatable(__ex);
    }
    return __ret;
}

#ifndef ICEE_PURE_CLIENT
bool
IceEDelegate::IceE::Object::ice_invoke(const string& operation,
                                      OperationMode mode,
				      const vector<Byte>& inParams,
				      vector<Byte>& outParams,
				      const Context& context)
{
    Outgoing __outS(__connection.get(), __reference.get(), operation, mode, context);
    BasicStream* __os = __outS.os();
    __os->writeBlob(inParams);
    bool ok = __outS.invoke();
    if(__reference->getMode() == Reference::ModeTwoway)
    {
        try
        {
            BasicStream* __is = __outS.is();
            Int sz = __is->getReadEncapsSize();
            __is->readBlob(outParams, sz);
        }
        catch(const ::IceE::LocalException& __ex)
        {
            throw ::IceEInternal::NonRepeatable(__ex);
        }
    }
    return ok;
}
#endif

ConnectionPtr
IceEDelegate::IceE::Object::ice_connection()
{
    return __connection;
}

void
IceEDelegate::IceE::Object::__copyFrom(const ::IceEInternal::Handle< ::IceEDelegate::IceE::Object>& from)
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
}

void
IceEDelegate::IceE::Object::setup(const ReferencePtr& ref)
{
    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!__reference);
    assert(!__connection);

    __reference = ref;
    __connection = __reference->getConnection();
}

bool
IceE::proxyIdentityLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
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
IceE::proxyIdentityEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
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
IceE::proxyIdentityAndFacetLess(const ObjectPrx& lhs, const ObjectPrx& rhs)
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
IceE::proxyIdentityAndFacetEqual(const ObjectPrx& lhs, const ObjectPrx& rhs)
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
