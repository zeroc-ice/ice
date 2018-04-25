// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

["objc:prefix:TestStream"]
module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
}

interface MyInterface;
class MyClass;

["cpp:comparable"] struct SmallStruct
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
    MyInterface* p;
}

["cpp:class"] struct ClassStruct
{
    int i;
}

class OptionalClass
{
    bool bo;
    byte by;
    optional(1) short sh;
    optional(2) int i;
}

sequence<MyEnum> MyEnumS;
sequence<SmallStruct> SmallStructS;
sequence<MyClass> MyClassS;

sequence<Ice::BoolSeq> BoolSS;
sequence<Ice::ByteSeq> ByteSS;
sequence<Ice::ShortSeq> ShortSS;
sequence<Ice::IntSeq> IntSS;
sequence<Ice::LongSeq> LongSS;
sequence<Ice::FloatSeq> FloatSS;
sequence<Ice::DoubleSeq> DoubleSS;
sequence<Ice::StringSeq> StringSS;
sequence<MyEnumS> MyEnumSS;
sequence<SmallStructS> SmallStructSS;
sequence<MyClassS> MyClassSS;

dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<long, float> LongFloatD;
dictionary<string, string> StringStringD;
dictionary<string, MyClass> StringMyClassD;

class MyClass
{
    MyClass c;
    Object o;
    SmallStruct s;
    Ice::BoolSeq seq1;
    Ice::ByteSeq seq2;
    Ice::ShortSeq seq3;
    Ice::IntSeq seq4;
    Ice::LongSeq seq5;
    Ice::FloatSeq seq6;
    Ice::DoubleSeq seq7;
    Ice::StringSeq seq8;
    MyEnumS seq9;
    MyClassS seq10;
    StringMyClassD d;
}

interface MyInterface
{
}

exception MyException
{
    MyClass c;
}

["objc:prefix:TestStreamSub"]
module Sub
{
    enum NestedEnum
    {
        nestedEnum1,
        nestedEnum2,
        nestedEnum3
    }

    ["cpp:comparable"] struct NestedStruct
    {
        bool bo;
        byte by;
        short sh;
        int i;
        long l;
        float f;
        double d;
        string str;
        NestedEnum e;
    }

    ["cpp:class"] struct NestedClassStruct
    {
        int i;
    }

    exception NestedException
    {
        string str;
    }
}
}

["objc:prefix:TestStream2"]
module Test2
{
["objc:prefix:TestStream2Sub2"]
module Sub2
{
    enum NestedEnum2
    {
        nestedEnum4,
        nestedEnum5,
        nestedEnum6
    }

    ["cpp:comparable"] struct NestedStruct2
    {
        bool bo;
        byte by;
        short sh;
        int i;
        long l;
        float f;
        double d;
        string str;
        NestedEnum2 e;
    }

    ["cpp:class"] struct NestedClassStruct2
    {
        int i;
    }

    exception NestedException2
    {
        string str;
    }
}
}
