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

class CollectorFactory;
ICE_API void incRef(CollectorFactory*);
ICE_API void decRef(CollectorFactory*);
typedef IceInternal::Handle<CollectorFactory> CollectorFactoryPtr;

}

#endif
