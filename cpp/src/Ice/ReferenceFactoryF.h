// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_REFERENCE_FACTORY_F_H
#define ICE_REFERENCE_FACTORY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ReferenceFactory;
IceUtil::Shared* upCast(ReferenceFactory*);
typedef Handle<ReferenceFactory> ReferenceFactoryPtr;

}

#endif
