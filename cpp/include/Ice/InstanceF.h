// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_INSTANCE_F_H
#define ICE_INSTANCE_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class Instance;
ICE_API IceUtil::Shared* upCast(Instance*);
typedef IceInternal::Handle<Instance> InstancePtr;
}

#endif
