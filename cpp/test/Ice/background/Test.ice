// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["cpp:dll-export:TEST_API"]]

module Test
{

interface Background
{
    void op();
    void opWithPayload(Ice::ByteSeq seq);

    void shutdown();
}

interface BackgroundController
{
    void pauseCall(string call);
    void resumeCall(string call);

    void holdAdapter();
    void resumeAdapter();

    void initializeSocketOperation(int status);
    void initializeException(bool enable);

    void readReady(bool enable);
    void readException(bool enable);

    void writeReady(bool enable);
    void writeException(bool enable);

    void buffered(bool enable);
}

}
