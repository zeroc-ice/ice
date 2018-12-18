// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_DEFAULTS_AND_OVERRIDES_F_H
#define ICE_DEFAULTS_AND_OVERRIDES_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class DefaultsAndOverrides;
IceUtil::Shared* upCast(DefaultsAndOverrides*);
typedef Handle<DefaultsAndOverrides> DefaultsAndOverridesPtr;

}

#endif
