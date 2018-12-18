// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.Ice.dispatcher"]]
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
