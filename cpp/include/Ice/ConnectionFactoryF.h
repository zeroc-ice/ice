// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_FACTORY_F_H
#define ICE_CONNECTION_FACTORY_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class OutgoingConnectionFactory;
ICE_API IceUtil::Shared* upCast(OutgoingConnectionFactory*);
typedef IceInternal::Handle<OutgoingConnectionFactory> OutgoingConnectionFactoryPtr;

class IncomingConnectionFactory;
ICE_API IceUtil::Shared* upCast(IncomingConnectionFactory*);
typedef IceInternal::Handle<IncomingConnectionFactory> IncomingConnectionFactoryPtr;

}

#endif
