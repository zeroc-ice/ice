// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ProxyF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/EndpointIF.h>
#include <Ice/Endpoint.h>
#include <Ice/ObjectF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/OutgoingAsyncF.h>
//#include <Ice/RouterF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
//#include <Ice/LocatorF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
#include <Ice/Current.h>
#include <Ice/StreamF.h>
#include <Ice/CommunicatorF.h>
#include <iosfwd>

namespace IceProxy
{

namespace Ice
{

class Locator;
class Router;

}

}

namespace IceInternal
{

ICE_API ::IceProxy::Ice::Object* upCast(::IceProxy::Ice::Locator*);
ICE_API ::IceProxy::Ice::Object* upCast(::IceProxy::Ice::Router*);

class LocalExceptionWrapper;

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Router> RouterPrx;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Locator> LocatorPrx;

class LocalException;

ICE_API void ice_writeObjectPrx(const ::Ice::OutputStreamPtr&, const ObjectPrx&);
ICE_API void ice_readObjectPrx(const ::Ice::InputStreamPtr&, ObjectPrx&);

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

    ICE_DEPRECATED_API ::Ice::Int ice_hash() const;
    ::Ice::Int ice_getHash() const;

    ICE_DEPRECATED_API ::Ice::CommunicatorPtr ice_communicator() const;
    ::Ice::CommunicatorPtr ice_getCommunicator() const;

    ::std::string ice_toString() const;

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

    // Returns true if ok, false if user exception.
    bool ice_invoke(const ::std::string& operation, 
                    ::Ice::OperationMode mode, 
                    const ::std::vector< ::Ice::Byte>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams)
    {
        const ::Ice::Context* context = 0;
        return ice_invoke(operation, mode, inParams, outParams, context);
    }
    bool ice_invoke(const ::std::string& operation, 
                    ::Ice::OperationMode mode, 
                    const ::std::vector< ::Ice::Byte>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams,
                    const ::Ice::Context& context)
    {
        return ice_invoke(operation, mode, inParams, outParams, &context);
    }
    bool ice_invoke(const ::std::string& operation, 
                    ::Ice::OperationMode mode, 
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams, 
                    ::std::vector< ::Ice::Byte>& outParams)
    {
        const ::Ice::Context* context = 0;
        return ice_invoke(operation, mode, inParams, outParams, context);
    }
    bool ice_invoke(const ::std::string& operation, 
                    ::Ice::OperationMode mode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>& inParams,
                    ::std::vector< ::Ice::Byte>& outParams,
                    const ::Ice::Context& context)
    {
        return ice_invoke(operation, mode, inParams, outParams, &context);
    }

    void ice_invoke_async(const ::Ice::AMI_Object_ice_invokePtr&, const ::std::string&, ::Ice::OperationMode,
                          const ::std::vector< ::Ice::Byte>&);
    void ice_invoke_async(const ::Ice::AMI_Object_ice_invokePtr&, const ::std::string&, ::Ice::OperationMode,
                          const ::std::vector< ::Ice::Byte>&, const ::Ice::Context&);
    void ice_invoke_async(const ::Ice::AMI_Array_Object_ice_invokePtr&, const ::std::string&, ::Ice::OperationMode,
                          const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&);
    void ice_invoke_async(const ::Ice::AMI_Array_Object_ice_invokePtr&, const ::std::string&, ::Ice::OperationMode, 
                          const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&, const ::Ice::Context&);

    ::Ice::Identity ice_getIdentity() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newIdentity(const ::Ice::Identity&) const;
    ::Ice::ObjectPrx ice_identity(const ::Ice::Identity&) const;

    ::Ice::Context ice_getContext() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newContext(const ::Ice::Context&) const;
    ::Ice::ObjectPrx ice_context(const ::Ice::Context&) const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_defaultContext() const;

    const ::std::string& ice_getFacet() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newFacet(const ::std::string&) const;
    ::Ice::ObjectPrx ice_facet(const ::std::string&) const;

    ::std::string ice_getAdapterId() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newAdapterId(const ::std::string&) const;
    ::Ice::ObjectPrx ice_adapterId(const ::std::string&) const;

    ::Ice::EndpointSeq ice_getEndpoints() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_newEndpoints(const ::Ice::EndpointSeq&) const;
    ::Ice::ObjectPrx ice_endpoints(const ::Ice::EndpointSeq&) const;

    ::Ice::Int ice_getLocatorCacheTimeout() const;
    ::Ice::ObjectPrx ice_locatorCacheTimeout(::Ice::Int) const;

    bool ice_isConnectionCached() const;
    ::Ice::ObjectPrx ice_connectionCached(bool) const;

    ::Ice::EndpointSelectionType ice_getEndpointSelection() const;
    ::Ice::ObjectPrx ice_endpointSelection(::Ice::EndpointSelectionType) const;

    bool ice_isSecure() const;
    ::Ice::ObjectPrx ice_secure(bool) const;

    bool ice_isPreferSecure() const;
    ::Ice::ObjectPrx ice_preferSecure(bool) const;

    ::Ice::RouterPrx ice_getRouter() const;
    ::Ice::ObjectPrx ice_router(const ::Ice::RouterPrx&) const;

    ::Ice::LocatorPrx ice_getLocator() const;
    ::Ice::ObjectPrx ice_locator(const ::Ice::LocatorPrx&) const;

    bool ice_isCollocationOptimized() const;
    ICE_DEPRECATED_API ::Ice::ObjectPrx ice_collocationOptimization(bool) const;
    ::Ice::ObjectPrx ice_collocationOptimized(bool) const;

    ::Ice::ObjectPrx ice_twoway() const;
    bool ice_isTwoway() const;
    ::Ice::ObjectPrx ice_oneway() const;
    bool ice_isOneway() const;
    ::Ice::ObjectPrx ice_batchOneway() const;
    bool ice_isBatchOneway() const;
    ::Ice::ObjectPrx ice_datagram() const;
    bool ice_isDatagram() const;
    ::Ice::ObjectPrx ice_batchDatagram() const;
    bool ice_isBatchDatagram() const;

    ::Ice::ObjectPrx ice_compress(bool) const;
    ::Ice::ObjectPrx ice_timeout(int) const;
    ::Ice::ObjectPrx ice_connectionId(const ::std::string&) const;

    bool ice_isThreadPerConnection() const;
    ::Ice::ObjectPrx ice_threadPerConnection(bool) const;

    ICE_DEPRECATED_API ::Ice::ConnectionPtr ice_connection();
    ::Ice::ConnectionPtr ice_getConnection();
    ::Ice::ConnectionPtr ice_getCachedConnection() const;

    ::IceInternal::ReferencePtr __reference() const;
    void __copyFrom(const ::Ice::ObjectPrx&);
    void __handleException(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>&, 
                           const ::Ice::LocalException&, int&);
    void __handleExceptionWrapper(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>&, 
                                  const ::IceInternal::LocalExceptionWrapper&);
    void __handleExceptionWrapperRelaxed(const ::IceInternal::Handle< ::IceDelegate::Ice::Object>&,
                                         const ::IceInternal::LocalExceptionWrapper&, int&);
    void __checkTwowayOnly(const char*) const;
    void __checkTwowayOnly(const ::std::string&) const;

    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __getDelegate();

protected:

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();

private:

    bool ice_isA(const ::std::string&, const ::Ice::Context*);
    void ice_ping(const ::Ice::Context*);
    ::std::vector< ::std::string> ice_ids(const ::Ice::Context*);
    ::std::string ice_id(const ::Ice::Context*);

   
    bool ice_invoke(const ::std::string&, 
                    ::Ice::OperationMode, 
                    const ::std::vector< ::Ice::Byte>&,
                    ::std::vector< ::Ice::Byte>&,
                    const ::Ice::Context*);
    bool ice_invoke(const ::std::string&, 
                    ::Ice::OperationMode,
                    const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                    ::std::vector< ::Ice::Byte>&,
                    const ::Ice::Context*);
  
    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceInternal::ProxyFactory;

    ::IceInternal::ReferencePtr _reference;
    ::IceInternal::Handle< ::IceDelegate::Ice::Object> _delegate;
};

} }

ICE_API ::std::ostream& operator<<(::std::ostream&, const ::IceProxy::Ice::Object&);

namespace IceDelegate { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared
{
public:

    virtual bool ice_isA(const ::std::string&, const ::Ice::Context*) = 0;
    virtual void ice_ping(const ::Ice::Context*) = 0;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Context*) = 0;
    virtual ::std::string ice_id(const ::Ice::Context*) = 0;
    virtual bool ice_invoke(const ::std::string&, ::Ice::OperationMode,
                            const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                            ::std::vector< ::Ice::Byte>&, const ::Ice::Context*) = 0;

    virtual ::Ice::ConnectionIPtr __getConnection(bool&) const = 0;
};

} }

namespace IceDelegateM { namespace Ice
{

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ~Object();

    virtual bool ice_isA(const ::std::string&, const ::Ice::Context*);
    virtual void ice_ping(const ::Ice::Context*);
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Context*);
    virtual ::std::string ice_id(const ::Ice::Context*);
    virtual bool ice_invoke(const ::std::string&, ::Ice::OperationMode, 
                            const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                            ::std::vector< ::Ice::Byte>&, const ::Ice::Context*);

    virtual ::Ice::ConnectionIPtr __getConnection(bool&) const;

    void __copyFrom(const ::IceInternal::Handle< ::IceDelegateM::Ice::Object>&);

protected:

    ::IceInternal::ReferencePtr __reference;
    ::Ice::ConnectionIPtr __connection;
    bool __compress;

private:

    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceProxy::Ice::Object;
};

} }

namespace IceDelegateD { namespace Ice
{

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual bool ice_isA(const ::std::string&, const ::Ice::Context*);
    virtual void ice_ping(const ::Ice::Context*);
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Context*);
    virtual ::std::string ice_id(const ::Ice::Context*);
    virtual bool ice_invoke(const ::std::string&, ::Ice::OperationMode,
                            const ::std::pair<const ::Ice::Byte*, const ::Ice::Byte*>&,
                            ::std::vector< ::Ice::Byte>&, const ::Ice::Context*);

    virtual ::Ice::ConnectionIPtr __getConnection(bool&) const;

    void __copyFrom(const ::IceInternal::Handle< ::IceDelegateD::Ice::Object>&);

protected:

    ::IceInternal::ReferencePtr __reference;
    ::Ice::ObjectAdapterPtr __adapter;

    void __initCurrent(::Ice::Current&, const ::std::string&, ::Ice::OperationMode, const ::Ice::Context*);

private:

    void setup(const ::IceInternal::ReferencePtr&, const ::Ice::ObjectAdapterPtr&);
    friend class ::IceProxy::Ice::Object;
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
template<typename P> P 
checkedCastImpl(const ::Ice::ObjectPrx& b, const ::Ice::Context* context)
{
    P d = 0;
    if(b.get())
    {
        typedef typename P::element_type T;

        d = dynamic_cast<T*>(b.get());
        if(!d && (context == 0 ? 
                  b->ice_isA(T::ice_staticId()) :
                  b->ice_isA(T::ice_staticId(), *context)))
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
ICE_API ::Ice::ObjectPrx checkedCastImpl(const ::Ice::ObjectPrx&, const std::string&, const std::string&,
                                         const ::Ice::Context*);

//
// Specializations for P = ::Ice::ObjectPrx
// We have to use inline functions for broken compilers such as VC7.
//

template<> inline ::Ice::ObjectPrx 
checkedCastImpl< ::Ice::ObjectPrx>(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context* context)
{
    return checkedCastImpl(b, f, "::Ice::Object", context);
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
checkedCastImpl(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context* context)
{
    P d = 0;

    typedef typename P::element_type T;
    ::Ice::ObjectPrx bb = checkedCastImpl(b, f, T::ice_staticId(), context);

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
    Ice::Context* ctx = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceInternal::checkedCastHelper<P::element_type>(b, tag, ctx);
#else
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag, ctx);
#endif
}

template<typename P, typename Y> inline P 
checkedCast(const ::IceInternal::ProxyHandle<Y>& b, const ::Ice::Context& context)
{
    Y* tag = 0;
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return ::IceInternal::checkedCastHelper<P::element_type>(b, tag, &context);
#else
    return ::IceInternal::checkedCastHelper<typename P::element_type>(b, tag, &context);
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
    Ice::Context* ctx = 0;
    return ::IceInternal::checkedCastImpl<P>(b, f, ctx);
}

template<typename P> inline P 
checkedCast(const ::Ice::ObjectPrx& b, const std::string& f, const ::Ice::Context& context)
{
    return ::IceInternal::checkedCastImpl<P>(b, f, &context);
}

template<typename P> inline P 
uncheckedCast(const ::Ice::ObjectPrx& b, const std::string& f)
{
    return ::IceInternal::uncheckedCastImpl<P>(b, f);
}

#endif
