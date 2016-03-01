// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INCOMING_ASYNC_F_H
#define ICE_INCOMING_ASYNC_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class IncomingAsync;
ICE_API IceUtil::Shared* upCast(IncomingAsync*);
typedef IceInternal::Handle<IncomingAsync> IncomingAsyncPtr;

}

namespace Ice
{

class AMD_Object_ice_invoke;
ICE_API IceUtil::Shared* upCast(::Ice::AMD_Object_ice_invoke*);
typedef IceInternal::Handle<AMD_Object_ice_invoke> AMD_Object_ice_invokePtr;

}

#endif
