// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_H
#define ICE_PROXY_H

#include <Ice/ProxyF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/EmitterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Stream;

ICE_API void write(Stream*, const ::Ice::Object_pptr&);
ICE_API void read(Stream*, ::Ice::Object_pptr&);

}

namespace __IceProxy { namespace Ice
{

class ICE_API Object : virtual public ::__Ice::Shared, JTCMutex
{
public:

    bool _implements(const std::string&);

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;

    ::Ice::Object_pptr _twoway() const;
    ::Ice::Object_pptr _oneway() const;
    ::Ice::Object_pptr _datagram() const;
    ::Ice::Object_pptr _timeout(int) const;

    ::__Ice::Reference __reference() const;
    void __copyTo(Object*) const;

protected:

    Object();
    virtual ~Object();

    ::__Ice::Handle< ::__IceDelegate::Ice::Object> __getDelegate();
    virtual ::__Ice::Handle< ::__IceDelegateM::Ice::Object> __createDelegateM();

private:

    void setup(const ::__Ice::Reference&);

    ::__Ice::Reference reference_;
    ::__Ice::Handle< ::__IceDelegate::Ice::Object> delegate_;

    friend class ::__Ice::ProxyFactoryI;
};

} }

namespace __IceDelegate { namespace Ice
{

class ICE_API Object : virtual public ::__Ice::Shared, JTCMutex
{
public:

    virtual bool _implements(const std::string&) = 0;

protected:

    Object();
    virtual ~Object();

private:

    virtual void setup(const ::__Ice::Reference&) = 0;
    friend class ::__IceProxy::Ice::Object;
};

} }

namespace __IceDelegateM { namespace Ice
{

class ICE_API Object : virtual public ::__IceDelegate::Ice::Object
{
public:

    virtual bool _implements(const std::string&);

protected:

    Object();
    virtual ~Object();

    const ::__Ice::Emitter& __emitter(); // const...& for performance
    const ::__Ice::Reference& __reference(); // const...& for performance

private:

    ::__Ice::Emitter emitter_;
    ::__Ice::Reference reference_;

    virtual void setup(const ::__Ice::Reference&);
    friend class ::__IceProxy::Ice::Object;
};

} }

#endif
