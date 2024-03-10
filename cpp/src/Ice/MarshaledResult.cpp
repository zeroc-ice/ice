//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/MarshaledResult.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/ReplyStatus.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

MarshaledResult::MarshaledResult(MarshaledResult&& rhs) { _ostr.swap(rhs._ostr); }

MarshaledResult::MarshaledResult(const Current& current)
    : // currentProtocolEncoding because we're writing the protocol header.
      _ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding)
{
    _ostr.writeBlob(replyHdr, sizeof(replyHdr));
    _ostr.write(current.requestId);
    _ostr.write(replyOK);
}

MarshaledResult&
MarshaledResult::operator=(MarshaledResult&& rhs)
{
    _ostr.swap(rhs._ostr);
    return *this;
}
