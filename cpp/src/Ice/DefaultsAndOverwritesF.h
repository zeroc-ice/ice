// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_DEFAULTS_AND_OVERWRITES_F_H
#define ICE_DEFAULTS_AND_OVERWRITES_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class DefaultsAndOverwrites;
void incRef(DefaultsAndOverwrites*);
void decRef(DefaultsAndOverwrites*);
typedef Handle<DefaultsAndOverwrites> DefaultsAndOverwritesPtr;

}

#endif
