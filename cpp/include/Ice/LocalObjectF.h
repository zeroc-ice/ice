// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_F_H
#define ICE_LOCAL_OBJECT_F_H

#include <Ice/Handle.h>

namespace Ice { class LocalObject; }

namespace IceInternal
{

ICE_API void incRef(::Ice::LocalObject*);
ICE_API void decRef(::Ice::LocalObject*);

}

namespace Ice
{

typedef IceInternal::Handle< LocalObject > LocalObjectPtr;

}

#endif
