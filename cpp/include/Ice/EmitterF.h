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

class Emitter;
void incRef(Emitter*);
void decRef(Emitter*);
typedef IceInternal::Handle<Emitter> EmitterPtr;

class EmitterFactory;
void incRef(EmitterFactory*);
void decRef(EmitterFactory*);
typedef IceInternal::Handle<EmitterFactory> EmitterFactoryPtr;

}

#endif
