// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.timeout"]]
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
