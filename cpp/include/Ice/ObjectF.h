// **********************************************************************
//
// Copyright (c) 2003
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
