// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_F_H
#define ICE_OUTGOING_ASYNC_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace Ice
{

class AsyncResult;
ICE_API IceUtil::Shared* upCast(::Ice::AsyncResult*);
typedef IceInternal::Handle<AsyncResult> AsyncResultPtr;

}

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

class ProxyBatchOutgoingAsync;
ICE_API IceUtil::Shared* upCast(ProxyBatchOutgoingAsync*);
typedef IceInternal::Handle<ProxyBatchOutgoingAsync> ProxyBatchOutgoingAsyncPtr;

class ConnectionBatchOutgoingAsync;
ICE_API IceUtil::Shared* upCast(ConnectionBatchOutgoingAsync*);
typedef IceInternal::Handle<ConnectionBatchOutgoingAsync> ConnectionBatchOutgoingAsyncPtr;

class CommunicatorBatchOutgoingAsync;
ICE_API IceUtil::Shared* upCast(CommunicatorBatchOutgoingAsync*);
typedef IceInternal::Handle<CommunicatorBatchOutgoingAsync> CommunicatorBatchOutgoingAsyncPtr;

}

#endif
