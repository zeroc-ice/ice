//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"

using namespace std;
using namespace Test;

ByteSeq
TestIntfI::opByteSpan(ByteSeq dataIn, ByteSeq& dataOut, const Ice::Current&)
{
    dataOut = std::move(dataIn);
    return dataOut;
}

ShortSeq
TestIntfI::opShortSpan(ShortSeq dataIn, ShortSeq& dataOut, const Ice::Current&)
{
    dataOut = std::move(dataIn);
    return dataOut;
}

StringSeq
TestIntfI::opStringSpan(StringSeq dataIn, StringSeq& dataOut, const Ice::Current&)
{
    dataOut = std::move(dataIn);
    return dataOut;
}

ByteSeq
TestIntfI::opOptionalByteSpan(optional<ByteSeq> dataIn, ByteSeq& dataOut, const Ice::Current&)
{
    if (dataIn)
    {
        dataOut = *std::move(dataIn);
        return dataOut;
    }
    else
    {
        dataOut = {std::byte{42}};
        return dataOut;
    }
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
