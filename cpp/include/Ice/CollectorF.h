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
void incRef(CollectorI*);
void decRef(CollectorI*);
typedef __Ice::Handle<CollectorI> Collector;

class CollectorFactoryI;
void incRef(CollectorFactoryI*);
void decRef(CollectorFactoryI*);
typedef __Ice::Handle<CollectorFactoryI> CollectorFactory;

}

#endif
