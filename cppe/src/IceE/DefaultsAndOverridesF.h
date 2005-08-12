// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_DEFAULTS_AND_OVERRIDES_F_H
#define ICE_DEFAULTS_AND_OVERRIDES_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class DefaultsAndOverrides;
void incRef(DefaultsAndOverrides*);
void decRef(DefaultsAndOverrides*);
typedef Handle<DefaultsAndOverrides> DefaultsAndOverridesPtr;

}

#endif
