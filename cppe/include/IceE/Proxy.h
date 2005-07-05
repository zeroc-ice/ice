// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_H
#define ICEE_PROXY_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/ProxyF.h>
#include <IceE/ProxyFactoryF.h>
#include <IceE/ConnectionF.h>
#include <IceE/EndpointF.h>
#include <IceE/ObjectF.h>
#include <IceE/ObjectAdapterF.h>
#include <IceE/ReferenceF.h>
//#include <IceE/RouterF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
//#include <IceE/LocatorF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
#include <IceE/Current.h>

namespace IceEProxy
{

namespace IceE
{

#ifndef ICEE_NO_ROUTER
class Router;
#endif
#ifndef ICEE_NO_LOCATOR
class Locator;
#endif

}

}

namespace IceEInternal
{

#ifndef ICEE_NO_ROUTER
ICEE_API void incRef(::IceEProxy::IceE::Router*);
ICEE_API void decRef(::IceEProxy::IceE::Router*);
#endif

#ifndef ICEE_NO_LOCATOR
ICEE_API void incRef(::IceEProxy::IceE::Locator*);
ICEE_API void decRef(::IceEProxy::IceE::Locator*);
#endif

}

namespace IceE
{

#ifndef ICEE_NO_ROUTER
typedef ::IceEInternal::ProxyHandle< ::IceEProxy::IceE::Router> RouterPrx;
#endif
#ifndef ICEE_NO_LOCATOR
typedef ::IceEInternal::ProxyHandle< ::IceEProxy::IceE::Locator> LocatorPrx;
#endif

class LocalException;

typedef ::std::map< ::std::string, ::std::string> Context;

class __U__Context { };
ICEE_API void __write(::IceEInternal::BasicStream*, const Context&, __U__Context);
ICEE_API void __read(::IceEInternal::BasicStream*, Context&, __U__Context);

}

namespace IceEProxy { namespace IceE
{

#if defined(_MSC_VER) && (_MSC_VER == 1310)
// Work around for VC++ 7.1 bug
typedef ::std::map< ::std::string, ::std::string> Context;
#endif

class ICEE_API Object : public ::IceE::Shared, private ::IceE::Mutex
{
public:

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;
    bool operator<(const Object&) const;
    ::IceE::Int ice_hash() const;

    bool ice_isA(const ::std::string&);
    bool ice_isA(const ::std::string&, const ::IceE::Context&);
    void ice_ping();
    void ice_ping(const ::IceE::Context&);
    ::std::vector< ::std::string> ice_ids();
    ::std::vector< ::std::string> ice_ids(const ::IceE::Context&);
    ::std::string ice_id();
    ::std::string ice_id(const ::IceE::Context&);
#ifndef ICEE_PURE_CLIENT
    bool ice_invoke(const ::std::string&, ::IceE::OperationMode, const ::std::vector< ::IceE::Byte>&,
	            ::std::vector< ::IceE::Byte>&); // Returns true if ok, false if user exception.
    bool ice_invoke(const ::std::string&, ::IceE::OperationMode, const ::std::vector< ::IceE::Byte>&,
	            ::std::vector< ::IceE::Byte>&,
		    const ::IceE::Context&); // Returns true if ok, false if user exception.
#endif

    ::IceE::Identity ice_getIdentity() const;
    ::IceE::ObjectPrx ice_newIdentity(const ::IceE::Identity&) const;

    ::IceE::Context ice_getContext() const;
    ::IceE::ObjectPrx ice_newContext(const ::IceE::Context&) const;
    ::IceE::ObjectPrx ice_defaultContext() const;

    const ::std::string& ice_getFacet() const;
    ::IceE::ObjectPrx ice_newFacet(const ::std::string&) const;

    ::IceE::ObjectPrx ice_twoway() const;
    bool ice_isTwoway() const;
    ::IceE::ObjectPrx ice_oneway() const;
    bool ice_isOneway() const;
#ifndef ICEE_NO_BATCH
    ::IceE::ObjectPrx ice_batchOneway() const;
    bool ice_isBatchOneway() const;
#endif

    ::IceE::ObjectPrx ice_timeout(int) const;
#ifndef ICEE_NO_ROUTER
    ::IceE::ObjectPrx ice_router(const ::IceE::RouterPrx&) const;
#endif
#ifndef ICEE_NO_LOCATOR
    ::IceE::ObjectPrx ice_locator(const ::IceE::LocatorPrx&) const;
#endif
    ::IceE::ObjectPrx ice_default() const;

    ::IceE::ConnectionPtr ice_connection();

    ::IceEInternal::ReferencePtr __reference() const;
    void __copyFrom(const ::IceE::ObjectPrx&);
    void __handleException(const ::IceE::LocalException&, int&);
    void __rethrowException(const ::IceE::LocalException&);
    void __checkTwowayOnly(const char*) const;

    ::IceEInternal::Handle< ::IceEDelegate::IceE::Object> __getDelegate();

protected:

    virtual ::IceEInternal::Handle< ::IceEDelegate::IceE::Object> __createDelegate();

    const ::IceE::Context& __defaultContext() const;

private:

    void setup(const ::IceEInternal::ReferencePtr&);
    friend class ::IceEInternal::ProxyFactory;

    ::IceEInternal::ReferencePtr _reference;
    ::IceEInternal::Handle< ::IceEDelegate::IceE::Object> _delegate;
};

} }

namespace IceEDelegate { namespace IceE
{

class ICEE_API Object : public ::IceE::Shared
{
public:

    ~Object();

    bool ice_isA(const ::std::string&, const ::IceE::Context&);
    void ice_ping(const ::IceE::Context&);
    ::std::vector< ::std::string> ice_ids(const ::IceE::Context&);
    ::std::string ice_id(const ::IceE::Context&);
#ifndef ICEE_PURE_CLIENT
    bool ice_invoke(const ::std::string&, ::IceE::OperationMode, const ::std::vector< ::IceE::Byte>&,
			    ::std::vector< ::IceE::Byte>&, const ::IceE::Context&);
#endif

    ::IceE::ConnectionPtr ice_connection();

    void __copyFrom(const ::IceEInternal::Handle< ::IceEDelegate::IceE::Object>&);

protected:

    ::IceEInternal::ReferencePtr __reference;
    ::IceE::ConnectionPtr __connection;

private:

    void setup(const ::IceEInternal::ReferencePtr&);
    friend class ::IceEProxy::IceE::Object;
};

} }

namespace IceE
{

ICEE_API bool proxyIdentityLess(const ObjectPrx&, const ObjectPrx&);
ICEE_API bool proxyIdentityEqual(const ObjectPrx&, const ObjectPrx&);

ICEE_API bool proxyIdentityAndFacetLess(const ObjectPrx&, const ObjectPrx&);
ICEE_API bool proxyIdentityAndFacetEqual(const ObjectPrx&, const ObjectPrx&);

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

namespace IceEInternal
{

//
// checkedCast and uncheckedCast functions without facet:
//

//
// Out of line implementations
//
template<typename P> P 
checkedCastImpl(const ::IceE::ObjectPrx& b)
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
checkedCastImpl(const ::IceE::ObjectPrx& b, const ::IceE::Context& ctx)
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
uncheckedCastImpl(const ::IceE::ObjectPrx& b)
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
ICEE_API ::IceE::ObjectPrx checkedCastImpl(const ::IceE::ObjectPrx&, const std::string&, const std::string&);
ICEE_API ::IceE::ObjectPrx checkedCastImpl(const ::IceE::ObjectPrx&, const std::string&, const std::string&,
                                         const ::IceE::Context&);

//
// Specializations for P = ::IceE::ObjectPrx
// We have to use inline functions for broken compilers such as VC7.
//

template<> inline ::IceE::ObjectPrx 
checkedCastImpl< ::IceE::ObjectPrx>(const ::IceE::ObjectPrx& b, const std::string& f)
{
    return checkedCastImpl(b, f, "::IceE::Object");
}

template<> inline ::IceE::ObjectPrx 
checkedCastImpl< ::IceE::ObjectPrx>(const ::IceE::ObjectPrx& b, const std::string& f, const ::IceE::Context& ctx)
{
    return checkedCastImpl(b, f, "::IceE::Object", ctx);
}

template<> inline ::IceE::ObjectPrx 
uncheckedCastImpl< ::IceE::ObjectPrx>(const ::IceE::ObjectPrx& b, const std::string& f)
{
    ::IceE::ObjectPrx d = 0;
    if(b)
    {
	d = b->ice_newFacet(f);
    }
    return d;
}

template<typename P> P 
checkedCastImpl(const ::IceE::ObjectPrx& b, const std::string& f)
{
    P d = 0;

    typedef typename P::element_type T;
    ::IceE::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId());

    if(bb)
    {
	d = new T;
	d->__copyFrom(bb);
    }
    return d;
}

template<typename P> P 
checkedCastImpl(const ::IceE::ObjectPrx& b, const std::string& f, const ::IceE::Context& ctx)
{
    P d = 0;

    typedef typename P::element_type T;
    ::IceE::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId(), ctx);

    if(bb)
    {
	d = new T;
	d->__copyFrom(bb);
    }
    return d;
}

template<typename P> P 
uncheckedCastImpl(const ::IceE::ObjectPrx& b, const std::string& f)
{
    P d = 0;
    if(b)
    {
	typedef typename P::element_type T;

	::IceE::ObjectPrx bb = b->ice_newFacet(f);
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
checkedCast(const ::IceEInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceEInternal::checkedCastHelper<P::element_type>(b, tag);
#else
    return ::IceEInternal::checkedCastHelper<typename P::element_type>(b, tag);
#endif
}

template<typename P, typename Y> inline P 
checkedCast(const ::IceEInternal::ProxyHandle<Y>& b, const ::IceE::Context& ctx)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceEInternal::checkedCastHelper<P::element_type>(b, tag, ctx);
#else
    return ::IceEInternal::checkedCastHelper<typename P::element_type>(b, tag, ctx);
#endif
}

template<typename P, typename Y> inline P
uncheckedCast(const ::IceEInternal::ProxyHandle<Y>& b)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceEInternal::uncheckedCastHelper<P::element_type>(b, tag);
#else
    return ::IceEInternal::uncheckedCastHelper<typename P::element_type>(b, tag);
#endif
}

template<typename P> inline P 
checkedCast(const ::IceE::ObjectPrx& b, const std::string& f)
{
    return ::IceEInternal::checkedCastImpl<P>(b, f);
}

template<typename P> inline P 
checkedCast(const ::IceE::ObjectPrx& b, const std::string& f, const ::IceE::Context& ctx)
{
    return ::IceEInternal::checkedCastImpl<P>(b, f, ctx);
}

template<typename P> inline P 
uncheckedCast(const ::IceE::ObjectPrx& b, const std::string& f)
{
    return ::IceEInternal::checkedCastImpl<P>(b, f);
}

#endif
