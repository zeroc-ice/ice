// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_DEFAULTS_AND_OVERRIDES_F_H
#define ICE_DEFAULTS_AND_OVERRIDES_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class DefaultsAndOverrides;
void incRef(DefaultsAndOverrides*);
void decRef(DefaultsAndOverrides*);
typedef Handle<DefaultsAndOverrides> DefaultsAndOverridesPtr;

}

#endif
