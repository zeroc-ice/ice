// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EMITTER_F_H
#define ICE_EMITTER_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class EmitterFactory;
ICE_API void incRef(EmitterFactory*);
ICE_API void decRef(EmitterFactory*);
typedef IceInternal::Handle<EmitterFactory> EmitterFactoryPtr;

}

#endif
