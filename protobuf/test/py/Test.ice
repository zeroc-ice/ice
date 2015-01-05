// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

["python:protobuf:Test_pb2.Message"] sequence<byte> Message;

class MyClass
{
    void shutdown();

    Message opMessage(Message i, out Message o);

    ["amd"] Message opMessageAMD(Message i, out Message o);
};

// Remaining type definitions are there to verify that the generated
// code compiles correctly.

sequence<Message> SLS;
sequence<SLS> SLSS;
dictionary<int, Message> SLD;
dictionary<int, SLS> SLSD;
struct Foo
{
    Message SLmem;
    SLS SLSmem;
};

exception Bar
{
    Message SLmem;
    SLS SLSmem;
};

class Baz
{
    Message SLmem;
    SLS SLSmem;
};

};

#endif
