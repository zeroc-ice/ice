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
#include <Ice/ReferenceF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/Shared.h>

namespace _IceIntf { namespace Ice
{

class ICE_API ObjectI : virtual public ::_Ice::SimpleShared
{
public:

    bool operator==(const ObjectI&) const;
    bool operator!=(const ObjectI&) const;

    ::_Ice::Reference reference() const;

protected:

    ObjectI();
    virtual ~ObjectI();
    friend class ::_Ice::ObjectFactoryI; // May create ObjectIs

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);

    void setup(const ::_Ice::Reference&);
    ::_IceStub::Ice::Object _getStub();

    ::_Ice::Reference reference_;
};

} }

namespace _IceStub { namespace Ice
{

class ICE_API ObjectI : virtual public ::_Ice::SimpleShared
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);
};

} }

namespace _IceStubM { namespace Ice
{

class ICE_API ObjectI : virtual public ::_Ice::SimpleShared
{
public:

protected:

    ObjectI();
    virtual ~ObjectI();

private:

    ObjectI(const ObjectI&);
    void operator=(const ObjectI&);
};

} }

#endif
