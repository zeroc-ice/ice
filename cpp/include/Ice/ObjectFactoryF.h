// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OBJECT_FACTORY_F_H
#define ICE_OBJECT_FACTORY_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class ObjectFactoryI;
void ICE_API incRef(ObjectFactoryI*);
void ICE_API decRef(ObjectFactoryI*);
typedef __Ice::Handle<ObjectFactoryI> ObjectFactory;

}

#endif
