// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OBJECT_ADAPTER_FACTORY_F_H
#define ICEE_OBJECT_ADAPTER_FACTORY_F_H

#include <IceE/Handle.h>

namespace IceEInternal
{

class ObjectAdapterFactory;
void incRef(ObjectAdapterFactory*);
void decRef(ObjectAdapterFactory*);
typedef IceEInternal::Handle<ObjectAdapterFactory> ObjectAdapterFactoryPtr;

}

#endif
