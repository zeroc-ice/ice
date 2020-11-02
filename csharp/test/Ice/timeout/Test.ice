//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::Timeout
{

sequence<byte> ByteSeq;

interface Timeout
{
    void op();
    void sendData(ByteSeq seq);
    void sleep(int to);
    bool checkDeadline();
}

interface Controller
{
    void holdAdapter(int to);
    void resumeAdapter();
    void shutdown();
}

}
