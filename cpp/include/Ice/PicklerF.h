// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PICKLER_F_H
#define ICE_PICKLER_F_H

#include <Ice/Handle.h>

namespace Ice { class Pickler; }

namespace __Ice
{

void ICE_API incRef(::Ice::Pickler*);
void ICE_API decRef(::Ice::Pickler*);

}

namespace Ice
{

typedef __Ice::Handle<Pickler> Pickler_ptr;

}

#endif
