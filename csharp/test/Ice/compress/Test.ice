//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Compress
{

sequence<byte> ByteSeq;

exception MyException
{
    ByteSeq bytes;
}

interface TestIntf
{
    [compress(args)] void opCompressArgs(int size, ByteSeq p1);
    [compress(return)] ByteSeq opCompressReturn(int size);
    [compress(args, return)] ByteSeq opCompressArgsAndReturn(ByteSeq p1);

    void opWithUserException(int size);

    void shutdown();
}

}
