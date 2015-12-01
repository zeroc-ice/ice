// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
class OutgoingAsync;
class ProxyOutgoingAsyncBase;
class CommunicatorFlushBatchAsync;

#ifdef ICE_CPP11_MAPPING
typedef ::std::shared_ptr<OutgoingAsyncBase> OutgoingAsyncBasePtr;
typedef ::std::shared_ptr<OutgoingAsync> OutgoingAsyncPtr;
typedef ::std::shared_ptr<ProxyOutgoingAsyncBase> ProxyOutgoingAsyncBasePtr;
typedef ::std::shared_ptr<CommunicatorFlushBatchAsync> CommunicatorFlushBatchAsyncPtr;
#else
ICE_API IceUtil::Shared* upCast(OutgoingAsyncBase*);
typedef IceInternal::Handle<OutgoingAsyncBase> OutgoingAsyncBasePtr;

ICE_API IceUtil::Shared* upCast(OutgoingAsync*);
typedef IceInternal::Handle<OutgoingAsync> OutgoingAsyncPtr;

ICE_API IceUtil::Shared* upCast(ProxyOutgoingAsyncBase*);
typedef IceInternal::Handle<ProxyOutgoingAsyncBase> ProxyOutgoingAsyncBasePtr;

ICE_API IceUtil::Shared* upCast(CommunicatorFlushBatchAsync*);
typedef IceInternal::Handle<CommunicatorFlushBatchAsync> CommunicatorFlushBatchAsyncPtr;
#endif
}

#endif
