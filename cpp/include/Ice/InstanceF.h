// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INSTANCE_F_H
#define ICE_INSTANCE_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Instance;
ICE_API void incRef(Instance*);
ICE_API void decRef(Instance*);
typedef IceInternal::Handle<Instance> InstancePtr;

}

#endif
