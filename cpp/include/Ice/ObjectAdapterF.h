// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_F_H
#define ICE_OBJECT_ADAPTER_F_H

#include <Ice/Handle.h>

namespace Ice { class ObjectAdapter; }

namespace __Ice
{

void ICE_API incRef(::Ice::ObjectAdapter*);
void ICE_API decRef(::Ice::ObjectAdapter*);

}

namespace Ice
{

typedef __Ice::Handle<ObjectAdapter> ObjectAdapter_ptr;

}

#endif
