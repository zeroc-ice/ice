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

namespace _Ice
{

class ObjectFactoryI;
void ICE_API _incRef(ObjectFactoryI*);
void ICE_API _decRef(ObjectFactoryI*);
typedef _Ice::Handle<ObjectFactoryI> ObjectFactory;

}

#endif
