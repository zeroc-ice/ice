// Copyright (c) ZeroC, Inc.

#pragma once

#include "Ice/BuiltinSequences.ice"

// Suppress invalid metadata warnings which we expect this test to generate.
[["suppress-warning"]]

["cs:identifier:Ice.stream.Test"]
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

    struct LargeStruct
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

    dictionary<byte, bool> ByteBoolD;
    dictionary<short, int> ShortIntD;
    dictionary<long, float> LongFloatD;
    dictionary<string, string> StringStringD;
    dictionary<string, MyClass> StringMyClassD;

    ["cs:generic:List"]
    sequence<bool> BoolList;
    ["cs:generic:List"]
    sequence<byte> ByteList;
    ["cs:generic:List"]
    sequence<MyEnum> MyEnumList;
    ["cs:generic:List"]
    sequence<LargeStruct> LargeStructList;
    ["cs:generic:List"]
    sequence<MyClass> MyClassList;
    ["cs:generic:List"]
    sequence<MyInterface*> MyInterfaceProxyList;

    ["cs:generic:LinkedList"]
    sequence<short> ShortLinkedList;
    ["cs:generic:LinkedList"]
    sequence<int> IntLinkedList;
    ["cs:generic:LinkedList"]
    sequence<MyEnum> MyEnumLinkedList;
    ["cs:generic:LinkedList"]
    sequence<LargeStruct> LargeStructLinkedList;

    ["cs:generic:Stack"]
    sequence<long> LongStack;
    ["cs:generic:Stack"]
    sequence<float> FloatStack;
    ["cs:generic:Stack"]
    sequence<LargeStruct> LargeStructStack;
    ["cs:generic:Stack"]
    sequence<MyInterface*> MyInterfaceProxyStack;

    //
    // This will produce a warning and use the default
    // sequence mapping. The generic:Stack metadata cannot be use
    // with object sequences.
    //
    ["cs:generic:Stack"]
    sequence<Object> ObjectStack;

    //
    // This will produce a warning and use the default
    // sequence mapping. The generic:Stack metadata cannot be use
    // with object sequences.
    //
    ["cs:generic:Stack"]
    sequence<MyClass> MyClassStack;

    ["cs:generic:Queue"]
    sequence<double> DoubleQueue;
    ["cs:generic:Queue"]
    sequence<string> StringQueue;
    ["cs:generic:Queue"]
    sequence<LargeStruct> LargeStructQueue;

    ["cs:generic:List"]
    sequence<Ice::StringSeq> StringSList;
    ["cs:generic:Stack"]
    sequence<Ice::StringSeq> StringSStack;

    ["cs:generic:SortedDictionary"]
    dictionary<string, string> SortedStringStringD;

    class MyClass
    {
        MyClass c;
        Object o;
        LargeStruct s;
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
