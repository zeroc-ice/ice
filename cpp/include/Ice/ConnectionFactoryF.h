// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_CONNECTION_FACTORY_F_H
#define ICE_CONNECTION_FACTORY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class OutgoingConnectionFactory;
ICE_API void incRef(OutgoingConnectionFactory*);
ICE_API void decRef(OutgoingConnectionFactory*);
typedef IceInternal::Handle<OutgoingConnectionFactory> OutgoingConnectionFactoryPtr;

class IncomingConnectionFactory;
ICE_API void incRef(IncomingConnectionFactory*);
ICE_API void decRef(IncomingConnectionFactory*);
typedef IceInternal::Handle<IncomingConnectionFactory> IncomingConnectionFactoryPtr;

}

#endif
