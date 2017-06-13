// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.serialize"]]
module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
}

interface Initial;
class Base;

["java:serialVersionUID:1001"]
struct Struct1
{
    bool bo;
    byte by;
    short sh;
    int i;
    long l;
    float f;
    double d;
    string str;
    MyEnum e;
    Initial* p;
}

sequence<byte> ByteS;
sequence<int> IntS;
sequence<MyEnum> MyEnumS;
sequence<Base> BaseS;

dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<string, MyEnum> StringMyEnumD;
dictionary<string, Base> StringBaseD;

["java:serialVersionUID:1002"]
class Base
{
    Base b;
    Object o;
    Struct1 s;
    ByteS seq1;
    IntS seq2;
    MyEnumS seq3;
    BaseS seq4;
    ByteBoolD d1;
    ShortIntD d2;
    StringMyEnumD d3;
    StringBaseD d4;
}

["java:serialVersionUID:1003"]
class Derived extends Base
{
    Object* p;
}

["java:serialVersionUID:1004"]
exception Ex
{
    Struct1 s;
    Base b;
}

interface Initial
{
    ByteS getStruct1();
    ByteS getBase();
    ByteS getEx();
    void shutdown();
}

}
