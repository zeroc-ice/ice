// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/ProxyF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ConnectionF.h>
#include <Ice/EndpointF.h>
#include <Ice/ObjectF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ReferenceF.h>
//#include <Ice/RouterF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
//#include <Ice/LocatorF.h> // Can't include RouterF.h here, otherwise we have cyclic includes
#include <Ice/Current.h>

namespace IceProxy
{

namespace Ice
{

class Router;
class Locator;

}

}

namespace IceInternal
{

ICE_API void incRef(::IceProxy::Ice::Router*);
ICE_API void decRef(::IceProxy::Ice::Router*);

ICE_API void checkedCast(const ::Ice::ObjectPrx&, const ::std::string&, ProxyHandle< ::IceProxy::Ice::Router>&);
ICE_API void uncheckedCast(const ::Ice::ObjectPrx&, const ::std::string&, ProxyHandle< ::IceProxy::Ice::Router>&);

ICE_API void incRef(::IceProxy::Ice::Locator*);
ICE_API void decRef(::IceProxy::Ice::Locator*);

ICE_API void checkedCast(const ::Ice::ObjectPrx&, const ::std::string&, ProxyHandle< ::IceProxy::Ice::Locator>&);
ICE_API void uncheckedCast(const ::Ice::ObjectPrx&, const ::std::string&, ProxyHandle< ::IceProxy::Ice::Locator>&);

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Router> RouterPrx;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Locator> LocatorPrx;

class LocalException;
class LocationForward;

}
namespace IceProxy { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared, ::IceUtil::Mutex
{
public:

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;
    bool operator<(const Object&) const;
    ::Ice::Int ice_hash() const;

    bool ice_isA(const std::string&, const ::Ice::Context& = ::Ice::Context());
    void ice_ping(const ::Ice::Context& = ::Ice::Context());
    ::std::vector< ::std::string> ice_ids(const ::Ice::Context& = ::Ice::Context());
    ::std::string ice_id(const ::Ice::Context& = ::Ice::Context());
    ::std::vector< ::std::string> ice_facets(const ::Ice::Context& = ::Ice::Context());
    bool ice_invoke(const std::string&, bool, const std::vector< ::Ice::Byte>&, std::vector< ::Ice::Byte>&,
		    const ::Ice::Context& = ::Ice::Context()); // Returns true if ok, false if user exception.

    ::Ice::Identity ice_getIdentity() const;
    ::Ice::ObjectPrx ice_newIdentity(const ::Ice::Identity&) const;

    std::string ice_getFacet() const;
    ::Ice::ObjectPrx ice_newFacet(const std::string&) const;

    ::Ice::ObjectPrx ice_twoway() const;
    ::Ice::ObjectPrx ice_oneway() const;
    ::Ice::ObjectPrx ice_batchOneway() const;
    ::Ice::ObjectPrx ice_datagram() const;
    ::Ice::ObjectPrx ice_batchDatagram() const;
    ::Ice::ObjectPrx ice_secure(bool) const;
    ::Ice::ObjectPrx ice_compress(bool) const;
    ::Ice::ObjectPrx ice_timeout(int) const;
    ::Ice::ObjectPrx ice_router(const ::Ice::RouterPrx&) const;
    ::Ice::ObjectPrx ice_locator(const ::Ice::LocatorPrx&) const;
    ::Ice::ObjectPrx ice_default() const;

    void ice_flush(); // Flush batch messages

    ::IceInternal::ReferencePtr __reference() const;
    void __copyFrom(const ::Ice::ObjectPrx&);
    void __handleException(const ::Ice::LocalException&, int&);
    void __rethrowException(const ::Ice::LocalException&);
    void __locationForward(const ::Ice::LocationForward&);

    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __getDelegate();

protected:

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();

private:

    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceInternal::ProxyFactory;

    ::IceInternal::ReferencePtr _reference;
    ::IceInternal::Handle< ::IceDelegate::Ice::Object> _delegate;
};

} }

namespace IceDelegate { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared
{
public:

    virtual bool ice_isA(const std::string&, const ::Ice::Context&) = 0;
    virtual void ice_ping(const ::Ice::Context&) = 0;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Context&) = 0;
    virtual ::std::string ice_id(const ::Ice::Context&) = 0;
    virtual ::std::vector< ::std::string> ice_facets(const ::Ice::Context&) = 0;
    virtual bool ice_invoke(const std::string&, bool, const std::vector< ::Ice::Byte>&, std::vector< ::Ice::Byte>&,
			    const ::Ice::Context&) = 0;
    virtual void ice_flush() = 0;
};

} }

namespace IceDelegateM { namespace Ice
{

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ~Object();

    virtual bool ice_isA(const std::string&, const ::Ice::Context&);
    virtual void ice_ping(const ::Ice::Context&);
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Context&);
    virtual ::std::string ice_id(const ::Ice::Context&);
    virtual ::std::vector< ::std::string> ice_facets(const ::Ice::Context&);
    virtual bool ice_invoke(const std::string&, bool, const std::vector< ::Ice::Byte>&, std::vector< ::Ice::Byte>&,
			    const ::Ice::Context&);
    virtual void ice_flush();

    void __copyFrom(const ::IceInternal::Handle< ::IceDelegateM::Ice::Object>&);

protected:

    ::IceInternal::ConnectionPtr __connection;
    ::IceInternal::ReferencePtr __reference;

private:

    void setup(const ::IceInternal::ReferencePtr&);
    friend class ::IceProxy::Ice::Object;

    std::vector< ::IceInternal::EndpointPtr> filterEndpoints(const std::vector< ::IceInternal::EndpointPtr>&) const;
};

} }

namespace IceDelegateD { namespace Ice
{

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual bool ice_isA(const std::string&, const ::Ice::Context&);
    virtual void ice_ping(const ::Ice::Context&);
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Context&);
    virtual ::std::string ice_id(const ::Ice::Context&);
    virtual ::std::vector< ::std::string> ice_facets(const ::Ice::Context&);
    virtual bool ice_invoke(const std::string&, bool, const std::vector< ::Ice::Byte>&, std::vector< ::Ice::Byte>&,
			    const ::Ice::Context&);
    virtual void ice_flush();

    void __copyFrom(const ::IceInternal::Handle< ::IceDelegateD::Ice::Object>&);

protected:

    ::Ice::ObjectAdapterPtr __adapter;
    ::IceInternal::ReferencePtr __reference;

    void __initCurrent(::Ice::Current&, const std::string&, bool, const ::Ice::Context&);

private:

    void setup(const ::IceInternal::ReferencePtr&, const ::Ice::ObjectAdapterPtr&);
    friend class ::IceProxy::Ice::Object;
};

} }

#endif

