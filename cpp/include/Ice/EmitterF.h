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

class EmitterI;
void incRef(EmitterI*);
void decRef(EmitterI*);
typedef __Ice::Handle<EmitterI> Emitter;

class EmitterFactoryI;
void incRef(EmitterFactoryI*);
void decRef(EmitterFactoryI*);
typedef __Ice::Handle<EmitterFactoryI> EmitterFactory;

}

#endif
