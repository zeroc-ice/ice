// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

[["cpp:dll-export:TEST_API"]]

module Test
{
    interface Background
    {
        void op();
        void opWithPayload(Ice::ByteSeq seq);
<<<<<<< Updated upstream

        void shutdown();
    }

=======

        void shutdown();
    }

>>>>>>> Stashed changes
    interface BackgroundController
    {
        void pauseCall(string call);
        void resumeCall(string call);
<<<<<<< Updated upstream

        void holdAdapter();
        void resumeAdapter();

        void initializeSocketOperation(int status);
        void initializeException(bool enable);

        void readReady(bool enable);
        void readException(bool enable);

        void writeReady(bool enable);
        void writeException(bool enable);

=======

        void holdAdapter();
        void resumeAdapter();

        void initializeSocketOperation(int status);
        void initializeException(bool enable);

        void readReady(bool enable);
        void readException(bool enable);

        void writeReady(bool enable);
        void writeException(bool enable);

>>>>>>> Stashed changes
        void buffered(bool enable);
    }
}
