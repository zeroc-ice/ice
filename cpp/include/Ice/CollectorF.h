// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COLLECTOR_F_H
#define ICE_COLLECTOR_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class CollectorI;
void ICE_API incRef(CollectorI*);
void ICE_API decRef(CollectorI*);
typedef __Ice::Handle<CollectorI> Collector;

class CollectorFactoryI;
void ICE_API incRef(CollectorFactoryI*);
void ICE_API decRef(CollectorFactoryI*);
typedef __Ice::Handle<CollectorFactoryI> CollectorFactory;

class CollectorFactoryFactoryI;
void ICE_API incRef(CollectorFactoryFactoryI*);
void ICE_API decRef(CollectorFactoryFactoryI*);
typedef __Ice::Handle<CollectorFactoryFactoryI> CollectorFactoryFactory;

}

#endif
