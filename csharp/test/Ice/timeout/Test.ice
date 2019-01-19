//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["cs:namespace:Ice.timeout"]
module Test
{

sequence<byte> ByteSeq;

interface Timeout
{
    void op();
    void sendData(ByteSeq seq);
    void sleep(int to);
}

interface Controller
{
    void holdAdapter(int to);
    void resumeAdapter();
    void shutdown();
}

}
