// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STORM_TRACE_LEVELS_F_H
#define ICE_STORM_TRACE_LEVELS_F_H

#include <Ice/Handle.h>

namespace IceStorm
{

class TraceLevels;
void incRef(TraceLevels*);
void decRef(TraceLevels*);
typedef IceInternal::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IceStorm

#endif
