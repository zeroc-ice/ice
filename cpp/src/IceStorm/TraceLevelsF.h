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
typedef IceInternal::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IceStorm

//
// This is necessary so that TraceLevelsPtr can be forward declared
//
namespace IceInternal
{

void incRef(IceStorm::TraceLevels*);
void decRef(IceStorm::TraceLevels*);

} // End namespace IceInternal

#endif
