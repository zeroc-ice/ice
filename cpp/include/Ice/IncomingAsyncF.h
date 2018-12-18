// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_INCOMING_ASYNC_F_H
#define ICE_INCOMING_ASYNC_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class IncomingAsync;
#ifdef ICE_CPP11_MAPPING
using IncomingAsyncPtr = ::std::shared_ptr<IncomingAsync>;
#else
ICE_API IceUtil::Shared* upCast(IncomingAsync*);
typedef IceInternal::Handle<IncomingAsync> IncomingAsyncPtr;
#endif

}

#ifndef ICE_CPP11_MAPPING
namespace Ice
{

class AMD_Object_ice_invoke;
ICE_API IceUtil::Shared* upCast(::Ice::AMD_Object_ice_invoke*);
typedef IceInternal::Handle<AMD_Object_ice_invoke> AMD_Object_ice_invokePtr;

}
#endif

#endif
