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
#include <Ice/ReferenceDataF.h>
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

    ::__Ice::ReferenceData __referenceData() const;
    void __copyTo(ObjectI*) const;

protected:

    ObjectI();
    virtual ~ObjectI();

    ::__IceStub::Ice::Object __getStub();
    virtual ::__IceStubM::Ice::Object __createStubM();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::__Ice::ReferenceData&);

    ::__Ice::ReferenceData referenceData_;
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

    virtual void setup(const ::__Ice::ReferenceData&) = 0;

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

    ::__Ice::Emitter __emitter();
    const std::string& __identity();

private:

    ::__Ice::EmitterFactory factory_;
    ::__Ice::ReferenceData referenceData_;

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::__Ice::ReferenceData&);

    friend class ::Ice::ObjectI; // May create and setup ObjectIs
};

} }

#endif
