//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

using OutgoingAsyncBasePtr = ::std::shared_ptr<OutgoingAsyncBase>;
using OutgoingAsyncPtr = ::std::shared_ptr<OutgoingAsync>;
using ProxyOutgoingAsyncBasePtr = ::std::shared_ptr<ProxyOutgoingAsyncBase>;
using CommunicatorFlushBatchAsyncPtr = ::std::shared_ptr<CommunicatorFlushBatchAsync>;
}

#endif
