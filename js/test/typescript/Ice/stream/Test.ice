// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

//
// Suppress invalid metadata warnings
//
[["suppress-warning:invalid-metadata, deprecated", "js:es6-module"]]

#include <Ice/BuiltinSequences.ice>

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
    MyInterface* p;
}

class OptionalClass
{
    bool bo;
    byte by;
    optional(1) short sh;
    optional(2) int i;
}

sequence<MyEnum> MyEnumS;
sequence<MyClass> MyClassS;
sequence<MyInterface> MyInterfaceS;

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
sequence<MyInterfaceS> MyInterfaceSS;

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

exception MyException
{
    MyClass c;
}

interface MyInterface
{
}

}
