//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

using IncomingConnectionFactoryPtr = ::std::shared_ptr<IncomingConnectionFactory>;

}

#endif
