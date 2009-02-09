// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

module Test
{

["java:serializable:Serialize.Small"] sequence<byte> SerialSmall;
["java:serializable:Serialize.Large"] sequence<byte> SerialLarge;
["java:serializable:Serialize.Struct"] sequence<byte> SerialStruct;

["ami"] class MyClass
{
    void shutdown();

    SerialSmall opSerialSmallJava(SerialSmall i, out SerialSmall o);
    SerialLarge opSerialLargeJava(SerialLarge i, out SerialLarge o);
    SerialStruct opSerialStructJava(SerialStruct i, out SerialStruct o);
};

// Remaining type definitions are there to verify that the generated
// code compiles correctly.

sequence<SerialLarge> SLS;
sequence<SLS> SLSS;
dictionary<int, SerialLarge> SLD;
dictionary<int, SLS> SLSD;
struct Foo
{
    SerialLarge SLmem;
    SLS SLSmem;
};

exception Bar
{
    SerialLarge SLmem;
    SLS SLSmem;
};

class Baz
{
    SerialLarge SLmem;
    SLS SLSmem;
};

};

#endif
