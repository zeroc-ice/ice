// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INCOMING_CONNECTION_FACTORY_F_H
#define ICEE_INCOMING_CONNECTION_FACTORY_F_H

#include <IceE/Handle.h>

namespace IceEInternal
{

class IncomingConnectionFactory;
ICEE_API void incRef(IncomingConnectionFactory*);
ICEE_API void decRef(IncomingConnectionFactory*);
typedef IceEInternal::Handle<IncomingConnectionFactory> IncomingConnectionFactoryPtr;

}

#endif
