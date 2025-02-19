// Copyright (c) ZeroC, Inc.

#include "Ice/MarshaledResult.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/ReplyStatus.h"
#include "Protocol.h"

using namespace Ice;
using namespace IceInternal;

// currentProtocolEncoding because we're writing the protocol header.
MarshaledResult::MarshaledResult(const Current& current)
    : _ostr(current.adapter->getCommunicator(), currentProtocolEncoding)
{
    _ostr.writeBlob(replyHdr, sizeof(replyHdr));
    _ostr.write(current.requestId);
    _ostr.write(ReplyStatus::Ok);
}

MarshaledResult::~MarshaledResult() = default; // avoid weak vtable
