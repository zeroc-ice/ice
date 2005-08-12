// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_CONNECTION_FACTORY_F_H
#define ICE_OUTGOING_CONNECTION_FACTORY_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class OutgoingConnectionFactory;
ICE_API void incRef(OutgoingConnectionFactory*);
ICE_API void decRef(OutgoingConnectionFactory*);
typedef IceInternal::Handle<OutgoingConnectionFactory> OutgoingConnectionFactoryPtr;

}

#endif
