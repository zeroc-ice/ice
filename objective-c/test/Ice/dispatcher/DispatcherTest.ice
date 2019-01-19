//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

["objc:prefix:TestDispatcher"]
module Test
{

interface TestIntf
{
    void op();
    void sleep(int to);
    void opWithPayload(Ice::ByteSeq seq);
    void shutdown();
}

interface TestIntfController
{
    void holdAdapter();
    void resumeAdapter();
}

}
