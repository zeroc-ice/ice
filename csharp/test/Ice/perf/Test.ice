//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.perf"]
module Test
{

sequence<byte> ByteSeq;

const int ByteSeqSize = 1024000; // 1MB

interface Performance
{
    void sendBytes(ByteSeq bytes);
    ByteSeq receiveBytes(int size);
    void shutdown();
}

}
