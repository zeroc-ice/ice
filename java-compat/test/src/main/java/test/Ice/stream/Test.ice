// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["java:package:test.Ice.stream"]]
module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
}

class MyClass;
interface MyInterface;

["java:serializable:test.Ice.stream.Serialize.Small"] sequence<byte> SerialSmall;

struct SmallStruct
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
    MyClass c;
    MyInterface* p;
    SerialSmall ss;
}

struct Point
{
    int x;
    int y;
}

sequence<MyEnum> MyEnumS;
sequence<MyClass> MyClassS;
dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<string, MyClass> StringMyClassD;

class OptionalClass
{
    bool bo;
    byte by;
    optional(1) short sh;
    optional(2) int i;
    optional(3) SmallStruct sm;

    optional(4) MyEnumS enumS4;
    optional(5) MyClassS myClassS5;

    optional(6) ByteBoolD byteBoolD6;
    optional(7) ShortIntD shortIntD7;

    optional(8) MyEnum enum8;
    optional(9) MyClass class9;
    optional(10) StringMyClassD stringMyClassD10;
    optional(12) Ice::IntSeq intSeq12;
    optional(13) Ice::ByteSeq byteSeq13;
    optional(14) Ice::StringSeq stringSeq14;
    optional(15) Point p15;
}

sequence<Ice::BoolSeq> BoolSS;
sequence<Ice::ByteSeq> ByteSS;
sequence<Ice::ShortSeq> ShortSS;
sequence<Ice::IntSeq> IntSS;
sequence<Ice::LongSeq> LongSS;
sequence<Ice::FloatSeq> FloatSS;
sequence<Ice::DoubleSeq> DoubleSS;
sequence<Ice::StringSeq> StringSS;
sequence<MyEnumS> MyEnumSS;
sequence<MyClassS> MyClassSS;

dictionary<long, float> LongFloatD;
dictionary<string, string> StringStringD;

class Bar;

class MyClass
{
    MyClass c;
    MyInterface* prx;
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

}
