// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef HELLO_ICE
#define HELLO_ICE

[["cpp:include:Person.pb.h"]]

module Demo
{

["cpp:protobuf:tutorial::Person", "java:protobuf:tutorial.PersonPB.Person", "python:protobuf:Person_pb2.Person"]
sequence<byte> Person;

interface Hello
{
    idempotent void sayHello(Person p);
    void shutdown();
};

};

#endif
