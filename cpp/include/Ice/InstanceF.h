// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
