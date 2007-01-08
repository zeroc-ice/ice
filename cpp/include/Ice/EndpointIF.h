// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ENDPOINT_I_F_H
#define ICE_ENDPOINT_I_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class EndpointI;
ICE_API void incRef(IceInternal::EndpointI*);
ICE_API void decRef(IceInternal::EndpointI*);
typedef IceInternal::Handle<EndpointI> EndpointIPtr;

}

#endif
