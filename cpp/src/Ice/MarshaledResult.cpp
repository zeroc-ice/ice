//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/MarshaledResult.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/ReplyStatus.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

MarshaledResult::MarshaledResult(const Current& current) :
    // currentProtocolEncoding because we're writing the protocol header.
    ostr(make_shared<Ice::OutputStream>(current.adapter->getCommunicator(), Ice::currentProtocolEncoding))
{
    ostr->writeBlob(replyHdr, sizeof(replyHdr));
    ostr->write(current.requestId);
    ostr->write(replyOK);
}
