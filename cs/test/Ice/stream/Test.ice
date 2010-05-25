// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ICE
#define TEST_ICE

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

sequence<bool> BoolS;
sequence<byte> ByteS;
sequence<short> ShortS;
sequence<int> IntS;
sequence<long> LongS;
sequence<float> FloatS;
sequence<double> DoubleS;
sequence<string> StringS;
sequence<MyEnum> MyEnumS;
sequence<MyClass> MyClassS;

sequence<BoolS> BoolSS;
sequence<ByteS> ByteSS;
sequence<ShortS> ShortSS;
sequence<IntS> IntSS;
sequence<LongS> LongSS;
sequence<FloatS> FloatSS;
sequence<DoubleS> DoubleSS;
sequence<StringS> StringSS;
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
sequence<StringS> StringSList;
["clr:generic:Stack"]
sequence<StringS> StringSStack;

["clr:generic:SortedDictionary"]
dictionary<string, string> SortedStringStringD;
["clr:collection"]
dictionary<string, int> StringIntDCollection;

["clr:serializable:Serialize.Small"] sequence<byte> SerialSmall;

class MyClass
{
    MyClass c;
    Object o;
    SmallStruct s;
    BoolS seq1;
    ByteS seq2;
    ShortS seq3;
    IntS seq4;
    LongS seq5;
    FloatS seq6;
    DoubleS seq7;
    StringS seq8;
    MyEnumS seq9;
    MyClassS seq10;
    StringMyClassD d;
    SerialSmall ss;
};

interface MyInterface
{
};

exception MyException
{
    MyClass c;
};

};

#endif
