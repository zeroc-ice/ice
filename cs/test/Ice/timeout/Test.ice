// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

sequence<byte> ByteSeq;

interface Timeout
{
    ["ami"] void op();
    ["ami"] void sendData(ByteSeq seq);
    ["ami"] void sleep(int to);

    void holdAdapter(int to);

    void shutdown();
};

};

#endif
