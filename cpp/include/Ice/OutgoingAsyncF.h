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

#ifndef ICE_OUTGOING_ASYNC_F_H
#define ICE_OUTGOING_ASYNC_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class OutgoingAsync;
ICE_PROTOCOL_API void incRef(OutgoingAsync*);
ICE_PROTOCOL_API void decRef(OutgoingAsync*);
typedef IceInternal::Handle<OutgoingAsync> OutgoingAsyncPtr;

}

namespace Ice
{

class AMI_Object_ice_invoke;

}

namespace IceInternal
{

ICE_API void incRef(::Ice::AMI_Object_ice_invoke*);
ICE_API void decRef(::Ice::AMI_Object_ice_invoke*);

}

namespace Ice
{

typedef IceInternal::Handle<AMI_Object_ice_invoke> AMI_Object_ice_invokePtr;

}

#endif
