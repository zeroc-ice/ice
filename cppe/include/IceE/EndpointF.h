// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ENDPOINT_F_H
#define ICEE_ENDPOINT_F_H

#include <IceE/Handle.h>

namespace IceEInternal
{

class Endpoint;
ICEE_PROTOCOL_API void incRef(Endpoint*);
ICEE_PROTOCOL_API void decRef(Endpoint*);
typedef IceEInternal::Handle<Endpoint> EndpointPtr;

}

#endif
