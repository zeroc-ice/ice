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
#include <Ice/Shared.h>
#include <string>

namespace _IceIntf { namespace Ice
{

class ICE_API Object : virtual public ::_Ice::SimpleShared
{
public:

protected:

    Object();
    virtual ~Object();

private:

    // TODO: Only for getting started...
    std::string host_;
    int port_;
    std::string identity_;

    Object(const Object&);
    void operator=(const Object&);
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
