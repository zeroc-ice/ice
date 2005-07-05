// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INSTANCE_F_H
#define ICEE_INSTANCE_F_H

#include <IceE/Handle.h>

namespace IceEInternal
{

class Instance;
ICEE_API void incRef(Instance*);
ICEE_API void decRef(Instance*);
typedef IceEInternal::Handle<Instance> InstancePtr;

}

#endif
