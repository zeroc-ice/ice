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

namespace _Ice
{

class CollectorI;
void ICE_API _incRef(CollectorI*);
void ICE_API _decRef(CollectorI*);
typedef _Ice::Handle<CollectorI> Collector;

class CollectorFactoryI;
void ICE_API _incRef(CollectorFactoryI*);
void ICE_API _decRef(CollectorFactoryI*);
typedef _Ice::Handle<CollectorFactoryI> CollectorFactory;

class CollectorFactoryFactoryI;
void ICE_API _incRef(CollectorFactoryFactoryI*);
void ICE_API _decRef(CollectorFactoryFactoryI*);
typedef _Ice::Handle<CollectorFactoryFactoryI> CollectorFactoryFactory;

}

#endif
