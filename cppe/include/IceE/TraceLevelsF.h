// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_TRACE_LEVELS_F_H
#define ICEE_TRACE_LEVELS_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class TraceLevels;
void incRef(TraceLevels*);
void decRef(TraceLevels*);
typedef Handle<TraceLevels> TraceLevelsPtr;

}

#endif
