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

namespace Ice
{

class Object;
void _incRef(Object*);
void _decRef(Object*);
typedef Handle<Object> ObjectHandle;

class Stub;
void _incRef(Stub*);
void _decRef(Stub*);
typedef Handle<Stub> StubHandle;

// ----------------------------------------------------------------------
// Object
// ----------------------------------------------------------------------

class ICE_API Object : virtual public SimpleShared
{
public:

protected:

    Object();
    virtual ~Object();

private:

    Object(const Object&);
    void operator=(const Object&);
};

// ----------------------------------------------------------------------
// Stub
// ----------------------------------------------------------------------

class ICE_API Stub : virtual public SimpleShared
{
public:

protected:

    Stub();
    virtual ~Stub();

private:

    Stub(const Stub&);
    void operator=(const Stub&);
};

}

#endif
