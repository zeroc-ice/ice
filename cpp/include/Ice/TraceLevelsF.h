// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TRACE_LEVELS_F_H
#define ICE_TRACE_LEVELS_F_H

#include <Ice/Handle.h>

namespace _Ice
{

class TraceLevelsI;
void ICE_API _incRef(TraceLevelsI*);
void ICE_API _decRef(TraceLevelsI*);
typedef _Ice::Handle<TraceLevelsI> TraceLevels;

}

#endif
