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

namespace _Ice
{

class EmitterI;
void ICE_API _incRef(EmitterI*);
void ICE_API _decRef(EmitterI*);
typedef _Ice::Handle<EmitterI> Emitter;

class EmitterFactoryI;
void ICE_API _incRef(EmitterFactoryI*);
void ICE_API _decRef(EmitterFactoryI*);
typedef _Ice::Handle<EmitterFactoryI> EmitterFactory;

class EmitterFactoryFactoryI;
void ICE_API _incRef(EmitterFactoryFactoryI*);
void ICE_API _decRef(EmitterFactoryFactoryI*);
typedef _Ice::Handle<EmitterFactoryFactoryI> EmitterFactoryFactory;

}

#endif
