// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_SERVANT_FACTORY_MANAGER_F_H
#define ICE_SERVANT_FACTORY_MANAGER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ObjectFactoryManager;
void incRef(ObjectFactoryManager*);
void decRef(ObjectFactoryManager*);
typedef Handle<ObjectFactoryManager> ObjectFactoryManagerPtr;

}

#endif
