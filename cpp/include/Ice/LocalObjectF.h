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

namespace __Ice
{

void ICE_API incRef(::Ice::LocalObject*);
void ICE_API decRef(::Ice::LocalObject*);

}

namespace Ice
{

typedef __Ice::Handle< LocalObject > LocalObject_ptr;

}

#endif
