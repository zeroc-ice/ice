// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
ICE_API void incRef(ReferenceFactory*);
ICE_API void decRef(ReferenceFactory*);
typedef IceInternal::Handle<ReferenceFactory> ReferenceFactoryPtr;

}

#endif
