// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_F_H
#define ICE_OBJECT_ADAPTER_F_H

#include <Ice/Handle.h>

namespace Ice { class ObjectAdapterI; }

namespace __Ice
{

void ICE_API incRef(::Ice::ObjectAdapterI*);
void ICE_API decRef(::Ice::ObjectAdapterI*);

}

namespace Ice
{

typedef __Ice::Handle<ObjectAdapterI> ObjectAdapter;

}

#endif
