// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <Ice/Handle.h>

namespace Ice
{

class Object;

}

namespace IceInternal
{

ICE_API void incRef(::Ice::Object*);
ICE_API void decRef(::Ice::Object*);

}

namespace Ice
{

typedef IceInternal::Handle< Object > ObjectPtr;

}

#endif
