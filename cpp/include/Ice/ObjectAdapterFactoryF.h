// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_ADAPTER_FACTORY_F_H
#define ICE_OBJECT_ADAPTER_FACTORY_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class ObjectAdapterFactory;
void incRef(ObjectAdapterFactory*);
void decRef(ObjectAdapterFactory*);
typedef __Ice::Handle<ObjectAdapterFactory> ObjectAdapterFactory_ptr;

}

#endif
