// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EMITTER_F_H
#define ICE_EMITTER_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class Emitter;
void incRef(Emitter*);
void decRef(Emitter*);
typedef __Ice::Handle<Emitter> Emitter_ptr;

class EmitterFactory;
void incRef(EmitterFactory*);
void decRef(EmitterFactory*);
typedef __Ice::Handle<EmitterFactory> EmitterFactory_ptr;

}

#endif
