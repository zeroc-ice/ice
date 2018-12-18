// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
using OutgoingAsyncBasePtr = ::std::shared_ptr<OutgoingAsyncBase>;
using OutgoingAsyncPtr = ::std::shared_ptr<OutgoingAsync>;
using ProxyOutgoingAsyncBasePtr = ::std::shared_ptr<ProxyOutgoingAsyncBase>;
using CommunicatorFlushBatchAsyncPtr = ::std::shared_ptr<CommunicatorFlushBatchAsync>;
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
