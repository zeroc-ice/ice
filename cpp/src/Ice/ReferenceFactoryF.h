// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_FACTORY_F_H
#define ICE_REFERENCE_FACTORY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ReferenceFactory;
void incRef(ReferenceFactory*);
void decRef(ReferenceFactory*);
typedef Handle<ReferenceFactory> ReferenceFactoryPtr;

}

#endif
