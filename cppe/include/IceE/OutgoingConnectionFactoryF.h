// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_CONNECTION_FACTORY_F_H
#define ICEE_OUTGOING_CONNECTION_FACTORY_F_H

#include <IceE/Handle.h>

namespace IceEInternal
{

class OutgoingConnectionFactory;
ICEE_API void incRef(OutgoingConnectionFactory*);
ICEE_API void decRef(OutgoingConnectionFactory*);
typedef IceEInternal::Handle<OutgoingConnectionFactory> OutgoingConnectionFactoryPtr;

}

#endif
