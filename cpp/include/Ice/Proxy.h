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
class Current;

};

namespace IceProxy { namespace Ice
{

class ICE_API Object : public ::IceUtil::Shared, JTCMutex
{
public:

    bool operator==(const Object&) const;
    bool operator<(const Object&) const;
    ::Ice::Int ice_hash() const;

    bool ice_isA(const std::string&);
    void ice_ping();
    void ice_invokeIn(const std::string&, bool, const std::vector< ::Ice::Byte>&);

    std::string ice_getIdentity() const;
    ::Ice::ObjectPrx ice_newIdentity(const std::string&) const;

    std::string ice_getFacet() const;
    ::Ice::ObjectPrx ice_newFacet(const std::string&) const;

    ::Ice::ObjectPrx ice_twoway() const;
    ::Ice::ObjectPrx ice_oneway() const;
    ::Ice::ObjectPrx ice_batchOneway() const;
    ::Ice::ObjectPrx ice_datagram() const;
    ::Ice::ObjectPrx ice_batchDatagram() const;
    ::Ice::ObjectPrx ice_secure(bool) const;
    ::Ice::ObjectPrx ice_timeout(int) const;

    void ice_flush(); // Flush batch messages

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

    virtual bool ice_isA(const std::string&) = 0;
    virtual void ice_ping() = 0;
    virtual void ice_invokeIn(const std::string&, const std::vector< ::Ice::Byte>&) = 0;
    virtual void ice_flush() = 0;
};

} }

namespace IceDelegateM { namespace Ice
{

class ICE_API Object : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual bool ice_isA(const std::string&);
    virtual void ice_ping();
    virtual void ice_invokeIn(const std::string&, const std::vector< ::Ice::Byte>&);
    virtual void ice_flush();

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

    virtual bool ice_isA(const std::string&);
    virtual void ice_ping();
    virtual void ice_invokeIn(const std::string&, const std::vector< ::Ice::Byte>&);
    virtual void ice_flush();

protected:

    ::Ice::ObjectAdapterPtr __adapter;
    ::IceInternal::ReferencePtr __reference;

private:

    void setup(const ::IceInternal::ReferencePtr&, const ::Ice::ObjectAdapterPtr&);
    friend class ::IceProxy::Ice::Object;
};

} }

#endif
