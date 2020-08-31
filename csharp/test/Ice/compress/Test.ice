//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Compress
{

sequence<byte> ByteSeq;

exception MyException
{
    ByteSeq bytes;
}

interface TestIntf
{
    [compress:params] void opCompressParams(int size, ByteSeq p1);
    [compress:return] ByteSeq opCompressReturn(int size);
    [compress:params,return] void opCompressParamsAndReturn(ByteSeq p1, out ByteSeq p2);

    void opWithUserException(int size);

    void shutdown();
}

}
