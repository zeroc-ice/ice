// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Handle.h>

namespace IceInternal
{

class ReferenceFactory;
IceUtil::Shared* upCast(ReferenceFactory*);
typedef Handle<ReferenceFactory> ReferenceFactoryPtr;

}
