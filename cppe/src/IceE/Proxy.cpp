// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Communicator.h>
#include <IceE/Proxy.h>
#include <IceE/ProxyFactory.h>
#include <IceE/Outgoing.h>
#include <IceE/Connection.h>
#include <IceE/Reference.h>
#include <IceE/Instance.h>
#include <IceE/BasicStream.h>
#include <IceE/LocalException.h>
#ifdef ICEE_HAS_ROUTER
#   include <IceE/RouterInfo.h>
#   include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#   include <IceE/LocatorInfo.h>
#   include <IceE/Locator.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::__write(::IceInternal::BasicStream* __os, const ::Ice::Context& v, ::Ice::__U__Context)
{
    __os->writeSize(::Ice::Int(v.size()));
    ::Ice::Context::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
	__os->write(p->first);
	__os->write(p->second);
    }
}

void
Ice::__read(::IceInternal::BasicStream* __is, ::Ice::Context& v, ::Ice::__U__Context)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    while(sz--)
    {
	::std::pair<const  ::std::string, ::std::string> pair;
	__is->read(const_cast< ::std::string&>(pair.first));
	::Ice::Context::iterator __i = v.insert(v.end(), pair);
	__is->read(__i->second);
    }
}

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId)
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
	    if(bb->ice_isA(typeId))
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

::Ice::ObjectPrx
IceInternal::checkedCastImpl(const ObjectPrx& b, const string& f, const string& typeId, const Context& ctx)
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
	ObjectPrx bb = b->ice_facet(f);
	try
	{
	    if(bb->ice_isA(typeId, ctx))
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
    return _reference->toString();
}

bool
IceProxy::Ice::Object::ice_isA(const string& __id, const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        ConnectionPtr __connection;
	try
	{
	    __checkTwowayOnly("ice_isA");
	    static const string __operation("ice_isA");
	    __connection = ice_getConnection();
            Outgoing __og(__connection.get(), _reference.get(), __operation, ::Ice::Nonmutating, __context);
            BasicStream* __stream = __og.stream();
            try
            {
                __stream->write(__id, false);
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
                        __stream->throwException();
		    }
		    catch(const ::Ice::UserException& __ex)
		    {
		        throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
		    }
                }
                __stream->read(__ret);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return __ret;
	}
	catch(const LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__connection, __ex, __cnt);
	}
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
	catch(...)
	{
	    throw;
	}
#endif
    }
}

void
IceProxy::Ice::Object::ice_ping(const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        ConnectionPtr __connection;
	try
	{
            static const string __operation("ice_ping");
	    __connection = ice_getConnection();
            Outgoing __og(__connection.get(), _reference.get(), __operation, ::Ice::Nonmutating, __context);
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.stream();
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
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
	    return;
	}
	catch(const LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__connection, __ex, __cnt);
	}
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
	catch(...)
	{
	    throw;
	}
#endif
    }
}

vector<string>
IceProxy::Ice::Object::ice_ids(const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        ConnectionPtr __connection;
	try
	{
	    __checkTwowayOnly("ice_ids");
            static const string __operation("ice_ids");
	    __connection = ice_getConnection();
            Outgoing __og(__connection.get(), _reference.get(), __operation, ::Ice::Nonmutating, __context);
            vector<string> __ret;
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.stream();
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
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return __ret;
	}
	catch(const LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__connection, __ex, __cnt);
	}
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
	catch(...)
	{
	    throw;
	}
#endif
    }
}

string
IceProxy::Ice::Object::ice_id(const Context* __context)
{
    int __cnt = 0;
    while(true)
    {
        ConnectionPtr __connection;
	try
	{
	    __checkTwowayOnly("ice_id");
            static const string __operation("ice_id");
	    __connection = ice_getConnection();
            Outgoing __og(__connection.get(), _reference.get(), __operation, ::Ice::Nonmutating, __context);
            string __ret;
            bool __ok = __og.invoke();
            try
            {
                BasicStream* __is = __og.stream();
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
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
            catch(...)
            {
                throw;
            }
#endif
            return __ret;
	}
	catch(const LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__connection, __ex, __cnt);
	}
	catch(const LocalException& __ex)
	{
	    __handleException(__connection, __ex, __cnt);
	}
#if defined(_MSC_VER) && defined(_M_ARM) // ARM bug.
	catch(...)
	{
	    throw;
	}
#endif
    }
}

Context
IceProxy::Ice::Object::ice_getContext() const
{
    return *_reference->getContext();
}

ObjectPrx
IceProxy::Ice::Object::ice_context(const Context& newContext) const
{
    ObjectPrx proxy(new ::IceProxy::Ice::Object());
    proxy->setup(_reference->changeContext(newContext));
    return proxy;
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

#ifdef ICEE_HAS_ROUTER

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

#endif

#ifdef ICEE_HAS_LOCATOR

string
IceProxy::Ice::Object::ice_getAdapterId() const
{
    return _reference->getAdapterId();
}

ObjectPrx
IceProxy::Ice::Object::ice_adapterId(const string& adapterId) const
{
    ReferencePtr ref = _reference->changeAdapterId(adapterId);
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

#endif

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

ConnectionPtr
IceProxy::Ice::Object::ice_getConnection()
{
    ::IceUtil::Mutex::Lock sync(*this);

    if(!_connection)
    {
        _connection = _reference->getConnection();

        //
        // If this proxy is for a non-local object, and we are
        // using a router, then add this proxy to the router info
        // object.
        //
#ifdef ICEE_HAS_ROUTER
        RouterInfoPtr ri = _reference->getRouterInfo();
        if(ri)
        {
            ri->addProxy(this);
        }
#endif
    }
    return _connection;
}

ConnectionPtr
IceProxy::Ice::Object::ice_getCachedConnection() const
{
    ::IceUtil::Mutex::Lock sync(*this);
    return _connection;
}

void
IceProxy::Ice::Object::__copyFrom(const ObjectPrx& from)
{
    ReferencePtr ref;
    ConnectionPtr con;

    {
	::IceUtil::Mutex::Lock sync(*from.get());

	ref = from->_reference;
	con = from->_connection;
    }

    //
    // No need to synchronize "*this", as this operation is only
    // called upon initialization.
    //

    assert(!_reference);
    assert(!_connection);

    _reference = ref;
    _connection = con;
}

void
IceProxy::Ice::Object::__handleException(const ConnectionPtr& connection, const LocalException& ex, int& cnt)
{
    //
    // Only _connection needs to be mutex protected here.
    //
    {
        ::IceUtil::Mutex::Lock sync(*this);
        if(connection.get() == _connection.get())
        {
            _connection = 0;
        }
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
IceProxy::Ice::Object::__handleExceptionWrapper(const ConnectionPtr& connection, const LocalExceptionWrapper& ex)
{
    {
        IceUtil::Mutex::Lock sync(*this);
        if(connection.get() == _connection.get())
        {
            _connection = 0;
        }
    }

    if(!ex.retry())
    {
        ex.get()->ice_throw();
    }
}

void
IceProxy::Ice::Object::__handleExceptionWrapperRelaxed(const ConnectionPtr& connection,
                                                       const LocalExceptionWrapper& ex, 
                                                       int& cnt)
{
    if(!ex.retry())
    {
        __handleException(connection, *ex.get(), cnt);
    }
    else
    {
        IceUtil::Mutex::Lock sync(*this);
        if(connection.get() == _connection.get())
        {
            _connection = 0;
        }
    }
}

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

ReferenceMode
IceProxy::Ice::Object::getMode() const
{
    return _reference->getMode();
}

ObjectPrx
IceProxy::Ice::Object::changeMode(ReferenceMode newMode) const
{
    if(_reference->getMode() == newMode)
    {
	return ObjectPrx(const_cast< ::IceProxy::Ice::Object*>(this));
    }
    else
    {
	ObjectPrx proxy(new ::IceProxy::Ice::Object());
	proxy->setup(_reference->changeMode(newMode));
	return proxy;
    }
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
