// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
