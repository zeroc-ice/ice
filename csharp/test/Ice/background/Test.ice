//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Background
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

    void initializeException(bool enable);

    void readReady(bool enable);
    void readException(bool enable);

    void writeReady(bool enable);
    void writeException(bool enable);

    void buffered(bool enable);
}

}
