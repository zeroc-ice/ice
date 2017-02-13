// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.Ice.background"]]
module Test
{

interface Background
{
    void op();
    void opWithPayload(Ice::ByteSeq seq);

    void shutdown();
};

interface BackgroundController
{
    void pauseCall(string call);
    void resumeCall(string call);

    void holdAdapter();
    void resumeAdapter();

    void initializeSocketStatus(int status);
    void initializeException(bool enable);

    void readReady(bool enable);
    void readException(bool enable);

    void writeReady(bool enable);
    void writeException(bool enable);

    void buffered(bool enable);
};

};
