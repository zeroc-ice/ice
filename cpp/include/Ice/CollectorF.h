// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COLLECTOR_F_H
#define ICE_COLLECTOR_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Collector;
void incRef(Collector*);
void decRef(Collector*);
typedef IceInternal::Handle<Collector> CollectorPtr;

class CollectorFactory;
void incRef(CollectorFactory*);
void decRef(CollectorFactory*);
typedef IceInternal::Handle<CollectorFactory> CollectorFactoryPtr;

}

#endif
