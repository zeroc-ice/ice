// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_FACTORY_F_H
#define ICE_OBJECT_ADAPTER_FACTORY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ObjectAdapterFactory;
void incRef(ObjectAdapterFactory*);
void decRef(ObjectAdapterFactory*);
typedef IceInternal::Handle<ObjectAdapterFactory> ObjectAdapterFactoryPtr;

}

#endif
