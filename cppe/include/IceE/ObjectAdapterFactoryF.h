// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_FACTORY_F_H
#define ICE_OBJECT_ADAPTER_FACTORY_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class ObjectAdapterFactory;
void incRef(ObjectAdapterFactory*);
void decRef(ObjectAdapterFactory*);
typedef IceInternal::Handle<ObjectAdapterFactory> ObjectAdapterFactoryPtr;

}

#endif
