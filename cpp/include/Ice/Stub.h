// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STUB_H
#define ICE_STUB_H

#include <Ice/StubF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/EmitterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Stream;

}

namespace __IceIntf { namespace Ice
{

class ICE_API Object : virtual public ::__Ice::Shared, JTCMutex
{
public:

    bool operator==(const Object&) const;
    bool operator!=(const Object&) const;

    ::Ice::Object _twoway() const;
    ::Ice::Object _oneway() const;
    ::Ice::Object _datagram() const;
    ::Ice::Object _timeout(int) const;

    ::__Ice::Reference __reference() const;
    void __copyTo(Object*) const;

protected:

    Object();
    virtual ~Object();

    ::__Ice::Handle< ::__IceStub::Ice::Object> __getStub();
    virtual ::__Ice::Handle< ::__IceStubM::Ice::Object> __createStubM();

private:

    void setup(const ::__Ice::Reference&);

    ::__Ice::Reference reference_;
    ::__Ice::Handle< ::__IceStub::Ice::Object> stub_;

    friend class ::__Ice::ObjectFactoryI; // May create and setup Objects
};

} }

namespace __IceStub { namespace Ice
{

class ICE_API Object : virtual public ::__Ice::Shared, JTCMutex
{
public:

protected:

    Object();
    virtual ~Object();

private:

    virtual void setup(const ::__Ice::Reference&) = 0;

    friend class ::__IceIntf::Ice::Object; // May create and setup Objects
};

} }

namespace __IceStubM { namespace Ice
{

class ICE_API Object : virtual public ::__IceStub::Ice::Object
{
public:

    static void __write(::__Ice::Stream*, const ::Ice::Object&);
    static void __read(::__Ice::Stream*, ::Ice::Object&);

protected:

    Object();
    virtual ~Object();

    const ::__Ice::Emitter& __emitter(); // const...& for performance
    const ::__Ice::Reference& __reference(); // const...& for performance

private:

    ::__Ice::Emitter emitter_;
    ::__Ice::Reference reference_;

    virtual void setup(const ::__Ice::Reference&);

    friend class ::__IceIntf::Ice::Object; // May create and setup Objects
};

} }

#endif
