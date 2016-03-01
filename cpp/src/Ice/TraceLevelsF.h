// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TRACE_LEVELS_F_H
#define ICE_TRACE_LEVELS_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class TraceLevels;
ICE_API IceUtil::Shared* upCast(TraceLevels*);
typedef Handle<TraceLevels> TraceLevelsPtr;

}

#endif
