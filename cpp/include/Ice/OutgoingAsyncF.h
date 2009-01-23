// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_F_H
#define ICE_OUTGOING_ASYNC_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class OutgoingAsync;
ICE_API IceUtil::Shared* upCast(OutgoingAsync*);
typedef IceInternal::Handle<OutgoingAsync> OutgoingAsyncPtr;

class OutgoingAsyncMessageCallback;
ICE_API IceUtil::Shared* upCast(OutgoingAsyncMessageCallback*);
typedef IceInternal::Handle<OutgoingAsyncMessageCallback> OutgoingAsyncMessageCallbackPtr;

class BatchOutgoingAsync;
ICE_API IceUtil::Shared* upCast(BatchOutgoingAsync*);
typedef IceInternal::Handle<BatchOutgoingAsync> BatchOutgoingAsyncPtr;

}

namespace Ice
{

class AMI_Object_ice_invoke;
class AMI_Array_Object_ice_invoke;
class AMI_Object_ice_flushBatchRequests;

}

namespace IceInternal
{

ICE_API IceUtil::Shared* upCast(::Ice::AMI_Object_ice_invoke*);
ICE_API IceUtil::Shared* upCast(::Ice::AMI_Array_Object_ice_invoke*);
ICE_API IceUtil::Shared* upCast(::Ice::AMI_Object_ice_flushBatchRequests*);

}

namespace Ice
{

typedef IceInternal::Handle<AMI_Object_ice_invoke> AMI_Object_ice_invokePtr;
typedef IceInternal::Handle<AMI_Array_Object_ice_invoke> AMI_Array_Object_ice_invokePtr;
typedef IceInternal::Handle<AMI_Object_ice_flushBatchRequests> AMI_Object_ice_flushBatchRequestsPtr;

}

#endif
