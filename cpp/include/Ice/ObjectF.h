// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <Ice/Handle.h>

namespace Ice { class Object; }

namespace __Ice
{

void ICE_API incRef(::Ice::Object*);
void ICE_API decRef(::Ice::Object*);

}

namespace Ice
{

typedef __Ice::Handle< Object > Object_iptr;

}

#endif
