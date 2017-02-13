// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
};

class MyClass;

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
    MyClass* p;
};

class OptionalClass
{
    bool bo;
    byte by;
    optional(1) short sh;
    optional(2) int i;
};

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

["clr:generic:List"]
sequence<bool> BoolList;
["clr:generic:List"]
sequence<byte> ByteList;
["clr:generic:List"]
sequence<MyEnum> MyEnumList;
["clr:generic:List"]
sequence<SmallStruct> SmallStructList;
["clr:generic:List"]
sequence<MyClass> MyClassList;
["clr:generic:List"]
sequence<MyClass*> MyClassProxyList;

["clr:generic:LinkedList"]
sequence<short> ShortLinkedList;
["clr:generic:LinkedList"]
sequence<int> IntLinkedList;
["clr:generic:LinkedList"]
sequence<MyEnum> MyEnumLinkedList;
["clr:generic:LinkedList"]
sequence<SmallStruct> SmallStructLinkedList;

["clr:generic:Stack"]
sequence<long> LongStack;
["clr:generic:Stack"]
sequence<float> FloatStack;
["clr:generic:Stack"]
sequence<SmallStruct> SmallStructStack;
["clr:generic:Stack"]
sequence<MyClass*> MyClassProxyStack;

["clr:generic:Stack"]
sequence<Object*> ObjectProxyStack;

//
// This will produce a warning and use the default 
// sequence mapping. The generic:Stack metadata cannot be use
// with object sequences.
//
["clr:generic:Stack"]
sequence<Object> ObjectStack;

//
// This will produce a warning and use the default 
// sequence mapping. The generic:Stack metadata cannot be use
// with object sequences.
//
["clr:generic:Stack"]
sequence<MyClass> MyClassStack;

["clr:generic:Queue"]
sequence<double> DoubleQueue;
["clr:generic:Queue"]
sequence<string> StringQueue;
["clr:generic:Queue"]
sequence<SmallStruct> SmallStructQueue;

["clr:collection"]
sequence<bool> BoolCollection;
["clr:collection"]
sequence<int> IntCollection;
["clr:collection"]
sequence<string> StringCollection;
["clr:collection"]
sequence<MyEnum> MyEnumCollection;
["clr:collection"]
sequence<SmallStruct> SmallStructCollection;
["clr:collection"]
sequence<MyClass> MyClassCollection;

["clr:generic:List"]
sequence<Ice::StringSeq> StringSList;
["clr:generic:Stack"]
sequence<Ice::StringSeq> StringSStack;

#ifndef SILVERLIGHT
#  if COMPACT
["clr:generic:SortedList"]
dictionary<string, string> SortedStringStringD;
#  else
["clr:generic:SortedDictionary"]
dictionary<string, string> SortedStringStringD;
#  endif
#endif

["clr:collection"]
dictionary<string, int> StringIntDCollection;

#ifndef SILVERLIGHT
["clr:serializable:Serialize.Small"] sequence<byte> SerialSmall;
#endif

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
#if !defined(COMPACT) && !defined(SILVERLIGHT)
    SerialSmall ss;
#endif
};

interface MyInterface
{
};

exception MyException
{
    MyClass c;
};

};
