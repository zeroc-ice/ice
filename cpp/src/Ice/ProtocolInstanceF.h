// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
