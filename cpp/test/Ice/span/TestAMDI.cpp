// Copyright (c) ZeroC, Inc.

#include "TestAMDI.h"

using namespace std;
using namespace Test;

void
TestIntfAMDI::opByteSpanAsync(
    ByteSeq dataIn,
    function<void(span<const byte> returnValue, span<const byte> dataOut)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(dataIn, dataIn);
}

void
TestIntfAMDI::opShortSpanAsync(
    ShortSeq dataIn,
    function<void(span<const int16_t> returnValue, span<const int16_t> dataOut)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(dataIn, dataIn);
}

void
TestIntfAMDI::opStringSpanAsync(
    StringSeq dataIn,
    function<void(span<string> returnValue, span<string> dataOut)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(dataIn, dataIn);
}

void
TestIntfAMDI::opOptionalByteSpanAsync(
    optional<ByteSeq> dataIn,
    function<void(optional<span<const byte>> returnValue, optional<span<const byte>> dataOut)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(dataIn, dataIn);
}

void
TestIntfAMDI::opOptionalShortSpanAsync(
    optional<ShortSeq> dataIn,
    function<void(optional<span<const int16_t>> returnValue, optional<span<const int16_t>> dataOut)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    response(dataIn, dataIn);
}

void
TestIntfAMDI::opOptionalStringSpanAsync(
    optional<StringSeq> dataIn,
    function<void(optional<span<string>> returnValue, optional<span<string>> dataOut)> response,
    function<void(exception_ptr)>,
    const Ice::Current&)
{
    if (dataIn)
    {
        response(*dataIn, *dataIn);
    }
    else
    {
        response(nullopt, nullopt);
    }
}

void
TestIntfAMDI::shutdownAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
    response();
}
