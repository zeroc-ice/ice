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

// ----------------------------------------------------------------------
// Object
// ----------------------------------------------------------------------

class ICE_API Object : virtual public SimpleShared
{
public:

protected:

    Object();

private:

    Object(const Object&);
    void operator=(const Object&);
};

}

#endif
