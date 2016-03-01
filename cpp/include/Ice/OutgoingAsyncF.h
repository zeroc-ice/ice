// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class OutgoingAsyncBase;
ICE_API IceUtil::Shared* upCast(OutgoingAsyncBase*);
typedef IceInternal::Handle<OutgoingAsyncBase> OutgoingAsyncBasePtr;

class OutgoingAsync;
ICE_API IceUtil::Shared* upCast(OutgoingAsync*);
typedef IceInternal::Handle<OutgoingAsync> OutgoingAsyncPtr;

class ProxyOutgoingAsyncBase;
ICE_API IceUtil::Shared* upCast(ProxyOutgoingAsyncBase*);
typedef IceInternal::Handle<ProxyOutgoingAsyncBase> ProxyOutgoingAsyncBasePtr;

class CommunicatorFlushBatchAsync;
ICE_API IceUtil::Shared* upCast(CommunicatorFlushBatchAsync*);
typedef IceInternal::Handle<CommunicatorFlushBatchAsync> CommunicatorFlushBatchAsyncPtr;

}

#endif
