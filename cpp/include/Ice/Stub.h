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
#include <Ice/InstanceF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/Shared.h>

namespace _IceIntf { namespace Ice
{

class ICE_API Object : virtual public ::_Ice::SimpleShared
{
public:

protected:

    Object();
    virtual ~Object();

private:

    Object(const Object&);
    void operator=(const Object&);
    friend class ::_Ice::ObjectFactoryI; // May create Objects

    ::_Ice::Instance instance_;

    // TODO: Only for getting started...
    std::string host_;
    int port_;
    std::string identity_;
};

} }

namespace _IceStub { namespace Ice
{

class ICE_API Object : virtual public ::_Ice::SimpleShared
{
public:

protected:

    Object();
    virtual ~Object();

private:

    Object(const Object&);
    void operator=(const Object&);
};

} }

namespace _IceStubM { namespace Ice
{

class ICE_API Object : virtual public ::_Ice::SimpleShared
{
public:

protected:

    Object();
    virtual ~Object();

private:

    Object(const Object&);
    void operator=(const Object&);
};

} }

#endif
