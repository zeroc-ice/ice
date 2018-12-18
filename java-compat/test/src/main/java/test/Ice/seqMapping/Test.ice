// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.seqMapping"]]
module Test
{

["java:serializable:test.Ice.seqMapping.Serialize.Small"] sequence<byte> SerialSmall;
["java:serializable:test.Ice.seqMapping.Serialize.Large"] sequence<byte> SerialLarge;
["java:serializable:test.Ice.seqMapping.Serialize.Struct"] sequence<byte> SerialStruct;

interface MyClass
{
    void shutdown();

    SerialSmall opSerialSmallJava(SerialSmall i, out SerialSmall o);
    SerialLarge opSerialLargeJava(SerialLarge i, out SerialLarge o);
    SerialStruct opSerialStructJava(SerialStruct i, out SerialStruct o);
}

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
}

exception Bar
{
    SerialLarge SLmem;
    SLS SLSmem;
}

class Baz
{
    SerialLarge SLmem;
    SLS SLSmem;
}

}
