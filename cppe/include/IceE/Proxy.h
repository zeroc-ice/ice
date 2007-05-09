// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class Object : public ::IceUtil::Shared, private ::IceUtil::Mutex
{
public:

    ICE_API bool operator==(const Object&) const;
    ICE_API bool operator<(const Object&) const;

    ICE_DEPRECATED_API ::Ice::Int ice_hash() const
    {
	return ice_getHash();
    }
    ICE_API ::Ice::Int ice_getHash() const;

    ICE_DEPRECATED_API ::Ice::CommunicatorPtr ice_communicator() const
    {
	return ice_getCommunicator();
    }
    ICE_API ::Ice::CommunicatorPtr ice_getCommunicator() const;

    ICE_API ::std::string ice_toString() const;

    bool ice_isA(const ::std::string& typeId)
    {
        return ice_isA(typeId, 0);
    }
    bool ice_isA(const ::std::string& typeId, const ::Ice::Context& context)
    {
        return ice_isA(typeId, &context);
    }

    void ice_ping()
    {
        ice_ping(0);
    }
    void ice_ping(const ::Ice::Context& context)
    {
        ice_ping(&context);
    }
    
    ::std::vector< ::std::string> ice_ids()
    {
        return ice_ids(0);
    }
    ::std::vector< ::std::string> ice_ids(const ::Ice::Context& context)
    {
        return ice_ids(&context);
    }

    ::std::string ice_id()
    {
        return ice_id(0);
    }
    ::std::string ice_id(const ::Ice::Context& context)
    {
        return ice_id(&context);
    }

    ICE_API ::Ice::Identity ice_getIdentity() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newIdentity(const ::Ice::Identity& id) const
    {
	return ice_identity(id);
    }
    ICE_API ::Ice::ObjectPrx ice_identity(const ::Ice::Identity&) const;
    
    ICE_API ::Ice::Context ice_getContext() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newContext(const ::Ice::Context& ctx) const
    {
	return ice_context(ctx);
    }
    ICE_API ::Ice::ObjectPrx ice_context(const ::Ice::Context&) const;

    ICE_API const ::std::string& ice_getFacet() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newFacet(const ::std::string& facet) const
    {
	return ice_facet(facet);
    }
    ICE_API ::Ice::ObjectPrx ice_facet(const ::std::string&) const;

#ifdef ICEE_HAS_ROUTER
    ICE_API ::Ice::ObjectPrx ice_router(const ::Ice::RouterPrx&) const;
    ICE_API ::Ice::RouterPrx ice_getRouter() const;
#endif
#ifdef ICEE_HAS_LOCATOR
    ICE_API ::Ice::ObjectPrx ice_adapterId(const std::string&) const;
    ICE_API std::string ice_getAdapterId() const;
    ICE_API ::Ice::ObjectPrx ice_locator(const ::Ice::LocatorPrx&) const;
    ICE_API ::Ice::LocatorPrx ice_getLocator() const;
#endif

    ICE_API bool ice_isSecure() const;
    ICE_API ::Ice::ObjectPrx ice_secure(bool) const;
    
    ::Ice::ObjectPrx ice_twoway() const
    {
        return changeMode(IceInternal::ReferenceModeTwoway);
    }
    bool ice_isTwoway() const
    {
        return getMode() == IceInternal::ReferenceModeTwoway;
    }
    ::Ice::ObjectPrx ice_oneway() const
    {
        return changeMode(IceInternal::ReferenceModeOneway);
    }
    bool ice_isOneway() const
    {
        return getMode() == IceInternal::ReferenceModeOneway;
    }
    ::Ice::ObjectPrx ice_datagram() const
    {
        return changeMode(IceInternal::ReferenceModeDatagram);
    }
    bool ice_isDatagram() const
    {
        return getMode() == IceInternal::ReferenceModeDatagram;
    }

    ::Ice::ObjectPrx ice_batchOneway() const
    {
        return changeMode(IceInternal::ReferenceModeBatchOneway);
    }
    bool ice_isBatchOneway() const
    {
        return getMode() == IceInternal::ReferenceModeBatchOneway;
    }
    ::Ice::ObjectPrx ice_batchDatagram() const
    {
        return changeMode(IceInternal::ReferenceModeBatchDatagram);
    }
    bool ice_isBatchDatagram() const
    {
        return getMode() == IceInternal::ReferenceModeBatchDatagram;
    }

    ICE_API ::Ice::ObjectPrx ice_timeout(int) const;

    ICE_DEPRECATED_API ::Ice::ConnectionPtr ice_connection()
    {
	return ice_getConnection();
    }
    ICE_API ::Ice::ConnectionPtr ice_getConnection();
    ICE_API ::Ice::ConnectionPtr ice_getCachedConnection() const;

    ::IceInternal::ReferencePtr __reference() const
    {
	return _reference;
    }

    ICE_API void __copyFrom(const ::Ice::ObjectPrx&);
    ICE_API void __handleException(const ::Ice::ConnectionPtr&, const ::Ice::LocalException&, int&);
    ICE_API void __handleExceptionWrapper(const ::Ice::ConnectionPtr&, const ::IceInternal::LocalExceptionWrapper&);
    ICE_API void __handleExceptionWrapperRelaxed(const ::Ice::ConnectionPtr&, const ::IceInternal::LocalExceptionWrapper&, int&);
    ICE_API void __checkTwowayOnly(const char*) const;

protected:

    ::IceInternal::ReferencePtr _reference;

private:

    ICE_API bool ice_isA(const ::std::string&, const ::Ice::Context*);
    ICE_API void ice_ping(const ::Ice::Context*);
    ICE_API ::std::vector< ::std::string> ice_ids(const ::Ice::Context*);
    ICE_API ::std::string ice_id(const ::Ice::Context*);

    ICE_API IceInternal::ReferenceMode getMode() const;
    ICE_API ::Ice::ObjectPrx changeMode(IceInternal::ReferenceMode) const;

    void setup(const ::IceInternal::ReferencePtr& ref)
    {
	//
	// No need to synchronize "*this", as this operation is only
	// called upon initialization.
	//
	
	assert(!_reference);
	assert(!_connection);
	
	_reference = ref;
    }
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
// Inline comparison functions for proxies
//
template<typename T, typename U>
inline bool operator==(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    ::IceProxy::Ice::Object* l = lhs.__upCast();
    ::IceProxy::Ice::Object* r = rhs.__upCast();
    if(l && r)
    {
        return *l == *r;
    }
    else
    {
        return !l && !r;
    }   
}

template<typename T, typename U>
inline bool operator!=(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return !operator==(lhs, rhs);
}

template<typename T, typename U>
inline bool operator<(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    ::IceProxy::Ice::Object* l = lhs.__upCast();
    ::IceProxy::Ice::Object* r = rhs.__upCast();
    if(l && r)
    {
        return *l < *r;
    }
    else
    {
        return !l && r;
    }
}

template<typename T, typename U>
inline bool operator<=(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return lhs < rhs || lhs == rhs;
}

template<typename T, typename U>
inline bool operator>(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return !(lhs < rhs || lhs == rhs);
}

template<typename T, typename U>
inline bool operator>=(const ProxyHandle<T>& lhs, const ProxyHandle<U>& rhs)
{
    return !(lhs < rhs);
}

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
