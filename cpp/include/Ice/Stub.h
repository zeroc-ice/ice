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

#include <Ice/Shared.h>
#include <Ice/Handle.h>

namespace _IceIntf { namespace Ice { class Object; } }
namespace _IceStub { namespace Ice { class Object; } }
namespace _IceStubM { namespace Ice { class Object; } }

namespace Ice
{

void ICE_API _incRef(::_IceIntf::Ice::Object*);
void ICE_API _decRef(::_IceIntf::Ice::Object*);

void ICE_API _incRef(::_IceStub::Ice::Object*);
void ICE_API _decRef(::_IceStub::Ice::Object*);

void ICE_API _incRef(::_IceStubM::Ice::Object*);
void ICE_API _decRef(::_IceStubM::Ice::Object*);

typedef Handle< ::_IceIntf::Ice::Object > Object;

}

namespace _IceIntf { namespace Ice
{

class ICE_API Object : virtual public ::Ice::SimpleShared
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

namespace _IceStub { namespace Ice
{

class ICE_API Object : virtual public ::Ice::SimpleShared
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

class ICE_API Object : virtual public ::Ice::SimpleShared
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
