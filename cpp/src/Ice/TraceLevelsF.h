// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRACE_LEVELS_F_H
#define ICE_TRACE_LEVELS_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class TraceLevels;
void incRef(TraceLevels*);
void decRef(TraceLevels*);
typedef Handle<TraceLevels> TraceLevelsPtr;

}

#endif
