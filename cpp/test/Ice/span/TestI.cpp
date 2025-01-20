// Copyright (c) ZeroC, Inc.

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

optional<ByteSeq>
TestIntfI::opOptionalByteSpan(optional<ByteSeq> dataIn, optional<ByteSeq>& dataOut, const Ice::Current&)
{
    dataOut = std::move(dataIn);
    return dataOut;
}

optional<ShortSeq>
TestIntfI::opOptionalShortSpan(optional<ShortSeq> dataIn, optional<ShortSeq>& dataOut, const Ice::Current&)
{
    dataOut = std::move(dataIn);
    return dataOut;
}

optional<StringSeq>
TestIntfI::opOptionalStringSpan(optional<StringSeq> dataIn, optional<StringSeq>& dataOut, const Ice::Current&)
{
    dataOut = std::move(dataIn);
    return dataOut;
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
