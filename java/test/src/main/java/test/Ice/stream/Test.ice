//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    tag(1) short sh;
    tag(2) int i;
    tag(3) SmallStruct sm;

    tag(4) MyEnumS enumS4;
    tag(5) MyClassS myClassS5;

    tag(6) ByteBoolD byteBoolD6;
    tag(7) ShortIntD shortIntD7;

    tag(8) MyEnum enum8;
    tag(9) MyClass class9;
    tag(10) StringMyClassD stringMyClassD10;
    tag(12) Ice::IntSeq intSeq12;
    tag(13) Ice::ByteSeq byteSeq13;
    tag(14) Ice::StringSeq stringSeq14;
    tag(15) Point p15;
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

exception MyException
{
    MyClass c;
}

interface MyInterface
{
};

}
