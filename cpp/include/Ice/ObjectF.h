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

namespace Ice { class ObjectS; }

namespace __Ice
{

void ICE_API incRef(::Ice::ObjectS*);
void ICE_API decRef(::Ice::ObjectS*);

}

namespace Ice
{

typedef __Ice::Handle< ObjectS > Object;

}

#endif
