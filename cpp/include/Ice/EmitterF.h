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
void ICE_API incRef(EmitterI*);
void ICE_API decRef(EmitterI*);
typedef __Ice::Handle<EmitterI> Emitter;

class EmitterFactoryI;
void ICE_API incRef(EmitterFactoryI*);
void ICE_API decRef(EmitterFactoryI*);
typedef __Ice::Handle<EmitterFactoryI> EmitterFactory;

class EmitterFactoryFactoryI;
void ICE_API incRef(EmitterFactoryFactoryI*);
void ICE_API decRef(EmitterFactoryFactoryI*);
typedef __Ice::Handle<EmitterFactoryFactoryI> EmitterFactoryFactory;

}

#endif
