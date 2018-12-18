// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_PROTOCOL_INSTANCE_F_H
#define ICE_PROTOCOL_INSTANCE_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class ProtocolInstance;
ICE_API IceUtil::Shared* upCast(ProtocolInstance*);
typedef IceInternal::Handle<ProtocolInstance> ProtocolInstancePtr;

}

#endif
