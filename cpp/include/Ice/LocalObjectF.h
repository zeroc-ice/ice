// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_LOCAL_OBJECT_F_H
#define ICE_LOCAL_OBJECT_F_H

#include <Ice/Handle.h>

namespace Ice
{

class LocalObject;

}

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
