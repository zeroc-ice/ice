// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include <IceUtil/Shared.h>
#include <Ice/ProxyF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/EmitterF.h>
#include <Ice/ObjectF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ReferenceF.h>

namespace Ice
{

class LocalException;
class LocationForward;

};

namespace IceProxy { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared, JTCMutex
{
public:

    bool operator==(const Object&) const;
    bool operator<(const Object&) const;
    ::Ice::Int _ice_hash() const;

    bool _ice_isA(const std::string&);
    void _ice_ping();

    std::string _ice_getIdentity() const;
    ::Ice::ObjectPrx _ice_newIdentity(const std::string&) const;

    std::string _ice_getFacet() const;
    ::Ice::ObjectPrx _ice_newFacet(const std::string&) const;

    ::Ice::ObjectPrx _ice_twoway() const;
    ::Ice::ObjectPrx _ice_oneway() const;
    ::Ice::ObjectPrx _ice_batchOneway() const;
    ::Ice::ObjectPrx _ice_datagram() const;
    ::Ice::ObjectPrx _ice_batchDatagram() const;
    ::Ice::ObjectPrx _ice_secure(bool) const;
    ::Ice::ObjectPrx _ice_timeout(int) const;

    void _ice_flush(); // Flush batch messages

    ::IceInternal::ReferencePtr __reference() const;
    void __copyFrom(const ::Ice::ObjectPrx&);
    void __handleException(const ::Ice::LocalException&, int&);
    void __rethrowException(const ::Ice::LocalException&);
    void __locationForward(const ::Ice::LocationForward&);

protected:

    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __getDelegate();
    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();

private:

    void setup(const ::IceInternal::ReferencePtr&);
    friend ::IceInternal::ProxyFactory;

    ::IceInternal::ReferencePtr _reference;
    ::IceInternal::Handle< ::IceDelegate::Ice::Object> _delegate;
};

} }

namespace IceDelegate { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared
{
public:

    virtual bool _ice_isA(const std::string&) = 0;
    virtual void _ice_ping() = 0;
    virtual void _ice_flush() = 0;
};

} }

namespace IceDelegateM { namespace Ice
{

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual bool _ice_isA(const std::string&);
    virtual void _ice_ping();
    virtual void _ice_flush();

protected:

    ::IceInternal::EmitterPtr __emitter;
    ::IceInternal::ReferencePtr __reference;

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

    virtual bool _ice_isA(const std::string&);
    virtual void _ice_ping();
    virtual void _ice_flush();

protected:

    ::Ice::ObjectAdapterPtr __adapter;
    ::IceInternal::ReferencePtr __reference;

private:

    void setup(const ::IceInternal::ReferencePtr&, const ::Ice::ObjectAdapterPtr&);
    friend class ::IceProxy::Ice::Object;
};

} }

#endif
