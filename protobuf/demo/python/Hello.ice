// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

#ifndef HELLO_ICE
#define HELLO_ICE

module Demo
{

["python:protobuf:Person_pb2.Person"] sequence<byte> Person;

interface Hello
{
    idempotent void sayHello(Person p);
    void shutdown();
};

};


#endif
