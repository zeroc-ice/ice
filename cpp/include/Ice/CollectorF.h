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

class Collector;
void incRef(Collector*);
void decRef(Collector*);
typedef __Ice::Handle<Collector> Collector_ptr;

class CollectorFactory;
void incRef(CollectorFactory*);
void decRef(CollectorFactory*);
typedef __Ice::Handle<CollectorFactory> CollectorFactory_ptr;

}

#endif
