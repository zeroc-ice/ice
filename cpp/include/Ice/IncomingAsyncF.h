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

#ifndef ICE_INCOMING_ASYNC_F_H
#define ICE_INCOMING_ASYNC_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class IncomingAsync;
ICE_PROTOCOL_API void incRef(IncomingAsync*);
ICE_PROTOCOL_API void decRef(IncomingAsync*);
typedef IceInternal::Handle<IncomingAsync> IncomingAsyncPtr;

}

namespace Ice
{

class AMD_Object_ice_invoke;

}

namespace IceInternal
{

ICE_API void incRef(::Ice::AMD_Object_ice_invoke*);
ICE_API void decRef(::Ice::AMD_Object_ice_invoke*);

}

namespace Ice
{

typedef IceInternal::Handle<AMD_Object_ice_invoke> AMD_Object_ice_invokePtr;

}

#endif
