// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_ENDPOINT_F_H
#define ICE_ENDPOINT_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Endpoint;
ICE_PROTOCOL_API void incRef(Endpoint*);
ICE_PROTOCOL_API void decRef(Endpoint*);
typedef IceInternal::Handle<Endpoint> EndpointPtr;

}

#endif
