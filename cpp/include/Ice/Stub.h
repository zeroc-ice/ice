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

class ICE_API ObjectI : virtual public ::_Ice::Shared, JTCMutex
{
public:

    bool operator==(const ObjectI&) const;
    bool operator!=(const ObjectI&) const;

    ::_Ice::ReferenceData _referenceData() const;
    void _copyTo(ObjectI*) const;

protected:

    ObjectI();
    virtual ~ObjectI();

    ::_IceStub::Ice::Object _getStub();
    virtual ::_IceStubM::Ice::Object _createStubM();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::_Ice::ReferenceData&);

    ::_Ice::ReferenceData referenceData_;
    ::_IceStub::Ice::Object stub_;

    friend class ::_Ice::ObjectFactoryI; // May create and setup ObjectIs
};

}

namespace _IceStub { namespace Ice
{

class ICE_API ObjectI : virtual public ::_Ice::Shared, JTCMutex
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    virtual void setup(const ::_Ice::ReferenceData&) = 0;

    friend class ::Ice::ObjectI; // May create and setup ObjectIs
};

} }

namespace _IceStubM { namespace Ice
{

class ICE_API ObjectI : virtual public ::_IceStub::Ice::ObjectI
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

    ::_Ice::Emitter _emitter();

private:

    ::_Ice::ReferenceData referenceData_;
    ::_Ice::EmitterFactory factory_;

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::_Ice::ReferenceData&);

    friend class ::Ice::ObjectI; // May create and setup ObjectIs
};

} }

#endif
