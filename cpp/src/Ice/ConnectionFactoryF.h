// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
IceUtil::Shared* upCast(OutgoingConnectionFactory*);
typedef IceInternal::Handle<OutgoingConnectionFactory> OutgoingConnectionFactoryPtr;

class IncomingConnectionFactory;

#ifdef ICE_CPP11_MAPPING
using IncomingConnectionFactoryPtr = ::std::shared_ptr<IncomingConnectionFactory>;
#else
IceUtil::Shared* upCast(IncomingConnectionFactory*);
typedef IceInternal::Handle<IncomingConnectionFactory> IncomingConnectionFactoryPtr;
#endif

}

#endif
