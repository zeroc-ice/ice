//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include "Ice/BuiltinSequences.ice"

module Test
{
    interface TestIntf
    {
        void sleep(int ms);

        void sendPayload(Ice::ByteSeq seq);

        void shutdown();
    }

    interface TestIntfController
    {
        void holdAdapter();
        void resumeAdapter();
    }
}
