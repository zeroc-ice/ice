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

#include <Ice/ObjectFactoryF.h>
#include <Ice/EmitterF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ObjectHandle.h>
#include <Ice/Shared.h>
#include <Ice/Outgoing.h>

namespace Ice
{

class ICE_API ObjectI : virtual public ::__Ice::Shared, JTCMutex
{
public:

    bool operator==(const ObjectI&) const;
    bool operator!=(const ObjectI&) const;

    Object _oneway() const;
    Object _twoway() const;

    ::__Ice::Reference __reference() const;
    void __copyTo(ObjectI*) const;

protected:

    ObjectI();
    virtual ~ObjectI();

    ::__IceStub::Ice::Object __getStub();
    virtual ::__IceStubM::Ice::Object __createStubM();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::__Ice::Reference&);

    ::__Ice::Reference reference_;
    ::__IceStub::Ice::Object stub_;

    friend class ::__Ice::ObjectFactoryI; // May create and setup ObjectIs
};

}

namespace __IceStub { namespace Ice
{

class ICE_API ObjectI : virtual public ::__Ice::Shared, JTCMutex
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    virtual void setup(const ::__Ice::Reference&) = 0;

    friend class ::Ice::ObjectI; // May create and setup ObjectIs
};

} }

namespace __IceStubM { namespace Ice
{

class ICE_API ObjectI : virtual public ::__IceStub::Ice::ObjectI
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

    
    const ::__Ice::Emitter& __emitter(); // const...& for performance
    const ::__Ice::Reference& __reference(); // const...& for performance

private:

    ::__Ice::EmitterFactory factory_;
    ::__Ice::Reference reference_;

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::__Ice::Reference&);

    friend class ::Ice::ObjectI; // May create and setup ObjectIs
};

} }

#endif
