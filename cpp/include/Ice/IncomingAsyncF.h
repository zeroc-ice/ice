// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INCOMING_ASYNC_F_H
#define ICE_INCOMING_ASYNC_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class IncomingAsync;
ICE_API void incRef(IncomingAsync*);
ICE_API void decRef(IncomingAsync*);
typedef IceInternal::Handle<IncomingAsync> IncomingAsyncPtr;

}

namespace Ice
{

class AMD_Object_ice_invoke;
class AMD_Array_Object_ice_invoke;

}

namespace IceInternal
{

ICE_API void incRef(::Ice::AMD_Object_ice_invoke*);
ICE_API void decRef(::Ice::AMD_Object_ice_invoke*);
ICE_API void incRef(::Ice::AMD_Array_Object_ice_invoke*);
ICE_API void decRef(::Ice::AMD_Array_Object_ice_invoke*);

}

namespace Ice
{

typedef IceInternal::Handle<AMD_Object_ice_invoke> AMD_Object_ice_invokePtr;
typedef IceInternal::Handle<AMD_Array_Object_ice_invoke> AMD_Array_Object_ice_invokePtr;

}

#endif
