// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INSTANCE_F_H
#define ICE_INSTANCE_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class Instance;
void incRef(Instance*);
void decRef(Instance*);
typedef __Ice::Handle<Instance> Instance_ptr;

}

#endif
