// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_H
#define ICEE_PROXY_H

#include <IceE/ProxyF.h>
#include <IceE/ProxyFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/ReferenceF.h>
#include <IceE/CommunicatorF.h>
//#include <IceE/RouterF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
//#include <IceE/LocatorF.h> // Can't include RouterF.h here, otherwise we have cyclic includes

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Identity.h>
#include <IceE/OperationMode.h>
#include <IceE/Outgoing.h>

#ifdef ICEE_HAS_ROUTER

namespace IceProxy { namespace Ice
{

class Router;

} }

namespace IceInternal
{

ICE_API void incRef(::IceProxy::Ice::Router*);
ICE_API void decRef(::IceProxy::Ice::Router*);

class LocalExceptionWrapper;

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Router> RouterPrx;

}

#endif // ICEE_HAS_ROUTER

#ifdef ICEE_HAS_LOCATOR

namespace IceProxy { namespace Ice
{

class Locator;

} }

namespace IceInternal
{

ICE_API void incRef(::IceProxy::Ice::Locator*);
ICE_API void decRef(::IceProxy::Ice::Locator*);

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Locator> LocatorPrx;

}

#endif // ICEE_HAS_LOCATOR

namespace Ice
{

class LocalException;

typedef ::std::map< ::std::string, ::std::string> Context;

class __U__Context { };
ICE_API void __write(::IceInternal::BasicStream*, const Context&, __U__Context);
ICE_API void __read(::IceInternal::BasicStream*, Context&, __U__Context);

}

namespace IceProxy { namespace Ice
{

#if defined(_MSC_VER) && (_MSC_VER == 1310)
// Work around for VC++ 7.1 bug
typedef ::std::map< ::std::string, ::std::string> Context;
#endif

class ICE_API Object : public ::IceUtil::Shared, private ::IceUtil::Mutex
{
public:

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;
    bool operator<(const Object&) const;
    ::Ice::Int ice_hash() const;

    ::Ice::CommunicatorPtr ice_communicator() const;

    ::std::string ice_toString() const;

    bool ice_isA(const ::std::string&);
    bool ice_isA(const ::std::string&, const ::Ice::Context&);
    void ice_ping();
    void ice_ping(const ::Ice::Context&);
    ::std::vector< ::std::string> ice_ids();
    ::std::vector< ::std::string> ice_ids(const ::Ice::Context&);
    ::std::string ice_id();
    ::std::string ice_id(const ::Ice::Context&);

    ::Ice::Identity ice_getIdentity() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newIdentity(const ::Ice::Identity&) const;
    ::Ice::ObjectPrx ice_identity(const ::Ice::Identity&) const;
    
    ::Ice::Context ice_getContext() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newContext(const ::Ice::Context&) const;
    ::Ice::ObjectPrx ice_context(const ::Ice::Context&) const;
    ::Ice::ObjectPrx ice_defaultContext() const;

    const ::std::string& ice_getFacet() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newFacet(const ::std::string&) const;
    ::Ice::ObjectPrx ice_facet(const ::std::string&) const;

    ::Ice::ObjectPrx ice_twoway() const;
    bool ice_isTwoway() const;
    ::Ice::ObjectPrx ice_oneway() const;
    bool ice_isOneway() const;
#ifdef ICEE_HAS_BATCH
    ::Ice::ObjectPrx ice_batchOneway() const;
    bool ice_isBatchOneway() const;
#endif

    ::Ice::ObjectPrx ice_timeout(int) const;
#ifdef ICEE_HAS_ROUTER
    ::Ice::ObjectPrx ice_router(const ::Ice::RouterPrx&) const;
#endif
#ifdef ICEE_HAS_LOCATOR
    ::Ice::ObjectPrx ice_locator(const ::Ice::LocatorPrx&) const;
#endif
    ::Ice::ConnectionPtr ice_connection();

    ::IceInternal::ReferencePtr __reference() const;
    void __copyFrom(const ::Ice::ObjectPrx&);
    void __handleException(const ::Ice::LocalException&, int&);
    void __handleExceptionWrapper(const ::IceInternal::LocalExceptionWrapper&);
    void __handleExceptionWrapperRelaxed(const ::IceInternal::LocalExceptionWrapper&, int&);
    void __checkTwowayOnly(const char*) const;

protected:

    const ::Ice::Context& __defaultContext() const;

    ::IceInternal::ReferencePtr _reference;

private:

    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceInternal::ProxyFactory;

    ::Ice::ConnectionPtr _connection;
};

} }

namespace Ice
{

ICE_API bool proxyIdentityLess(const ObjectPrx&, const ObjectPrx&);
ICE_API bool proxyIdentityEqual(const ObjectPrx&, const ObjectPrx&);

ICE_API bool proxyIdentityAndFacetLess(const ObjectPrx&, const ObjectPrx&);
ICE_API bool proxyIdentityAndFacetEqual(const ObjectPrx&, const ObjectPrx&);

struct ProxyIdentityLess : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
	return proxyIdentityLess(lhs, rhs);
    }
};

struct ProxyIdentityEqual : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
	return proxyIdentityEqual(lhs, rhs);
    }
};

struct ProxyIdentityAndFacetLess : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
	return proxyIdentityAndFacetLess(lhs, rhs);
    }
};

struct ProxyIdentityAndFacetEqual : std::binary_function<bool, ObjectPrx&, ObjectPrx&>
{
    bool operator()(const ObjectPrx& lhs, const ObjectPrx& rhs) const
    {
	return proxyIdentityAndFacetEqual(lhs, rhs);
    }
};

}

namespace IceInternal
{

//
// checkedCast and uncheckedCast functions without facet:
//

//
// Out of line implementations
//
template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx& b)
{
    P d = 0;
    if(b.get())
    {
	typedef typename P::element_type T;

	d = dynamic_cast<T*>(b.get());
	if(!d && b->ice_isA(T::ice_staticId()))
	{
	    d = new T;
	    d->__copyFrom(b);
	}
    }
    return d;
}

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx& b, const ::Ice::Context& ctx)
{
    P d = 0;
    if(b.get())
    {
	typedef typename P::element_type T;

	d = dynamic_cast<T*>(b.get());
	if(!d && b->ice_isA(T::ice_staticId(), ctx))
	{
	    d = new T;
	    d->__copyFrom(b);
	}
    }
    return d;
}

template<typename P> P 
uncheckedCastImpl(const ::Ice::ObjectPrx& b)
{
    P d = 0;
    if(b)
    {
	typedef typename P::element_type T;

	d = dynamic_cast<T*>(b.get());
	if(!d)
	{
	    d = new T;
	    d->__copyFrom(b);
	}
    }
    return d;
}

// 
// checkedCast and uncheckedCast with facet:
//

//
// Helper with type ID.
//
ICE_API ::Ice::ObjectPrx checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&, const std::string&);
ICE_API ::Ice::ObjectPrx checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&, const std::string&,
                                         const ::Ice::Context&);

//
// Specializations for P = ::Ice::ObjectPrx
// We have to use inline functions for broken compilers such as VC7.
//

template<> inline ::Ice::ObjectPrx 
checkedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f)
{
    return checkedCastImpl(b, f, "::Ice::Object");
}

template<> inline ::Ice::ObjectPrx 
checkedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& ctx)
{
    return checkedCastImpl(b, f, "::Ice::Object", ctx);
}

template<> inline ::Ice::ObjectPrx 
uncheckedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f)
{
    ::Ice::ObjectPrx d = 0;
    if(b)
    {
	d = b->ice_facet(f);
    }
    return d;
}

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f)
{
    P d = 0;

    typedef typename P::element_type T;
    ::Ice::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId());

    if(bb)
    {
	d = new T;
	d->__copyFrom(bb);
    }
    return d;
}

template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& ctx)
{
    P d = 0;

    typedef typename P::element_type T;
    ::Ice::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId(), ctx);

    if(bb)
    {
	d = new T;
	d->__copyFrom(bb);
    }
    return d;
}

template<typename P> P 
uncheckedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f)
{
    P d = 0;
    if(b)
    {
	typedef typename P::element_type T;

	::Ice::ObjectPrx bb = b->ice_facet(f);
	d = new T;
	d->__copyFrom(bb);
    }
    return d;
}
}

//
// checkedCast and uncheckedCast functions provided in the global namespace
// 

template<typename P, typename Y> inline P 
checkedCast(const ::IceInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceInternal::checkedCastHelper<P::element_type>(b, tag);
#else
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag);
#endif
}

template<typename P, typename Y> inline P 
checkedCast(const ::IceInternal::ProxyHandle<Y>& b, const ::Ice::Context& ctx)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceInternal::checkedCastHelper<P::element_type>(b, tag, ctx);
#else
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag, ctx);
#endif
}

template<typename P, typename Y> inline P
uncheckedCast(const ::IceInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceInternal::uncheckedCastHelper<P::element_type>(b, tag);
#else
    return ::IceInternal::uncheckedCastHelper<typename P::element_type>(b, tag);
#endif
}

template<typename P> inline P 
checkedCast(const ::Ice::ObjectPrx& b, const std::string& f)
{
    return ::IceInternal::checkedCastImpl<P>(b, f);
}

template<typename P> inline P 
checkedCast(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& ctx)
{
    return ::IceInternal::checkedCastImpl<P>(b, f, ctx);
}

template<typename P> inline P 
uncheckedCast(const ::Ice::ObjectPrx& b, const std::string& f)
{
    return ::IceInternal::checkedCastImpl<P>(b, f);
}

#endif
