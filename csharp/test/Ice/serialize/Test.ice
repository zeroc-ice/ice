//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/BuiltinSequences.ice>

[["cs:typeid-namespace:Ice.serialize.TypeId"]]

["cs:namespace:Ice.serialize"]
module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
}

class MyClass;

struct ValStruct
{
    bool bo;
    byte by;
    short sh;
    int i;
    long l;
    MyEnum e;
}

interface MyInterface
{
    void op();
}

sequence<MyInterface*> ProxySeq;

["clr:property"]
struct RefStruct
{
    string s;
    string sp;
    MyClass c;
    MyInterface* p;
    ProxySeq seq;
}

sequence<ValStruct> ValStructS;
["clr:generic:List"]
sequence<ValStruct> ValStructList;
["clr:generic:LinkedList"]
sequence<ValStruct> ValStructLinkedList;
["clr:generic:Stack"]
sequence<ValStruct> ValStructStack;
["clr:generic:Queue"]
sequence<ValStruct> ValStructQueue;

dictionary<int, string> IntStringD;
dictionary<int, ValStruct> IntValStructD;
dictionary<int, MyInterface*> IntProxyD;
["clr:generic:SortedDictionary"]
dictionary<int, string> IntStringSD;

class Base
{
    bool bo;
    byte by;
    short sh;
    int i;
    long l;
    MyEnum e;
}

class MyClass extends Base
{
    MyClass c;
    Object o;
    ValStruct s;
}

exception MyException
{
    string name;
    byte b;
    short s;
    int i;
    long l;
    ValStruct vs;
    RefStruct rs;
    MyClass c;
    MyInterface* p;

    ValStructS vss;
    ValStructList vsl;
    ValStructLinkedList vsll;
    ValStructStack vssk;
    ValStructQueue vsq;

    IntStringD isd;
    IntValStructD ivd;
    IntProxyD ipd;
    IntStringSD issd;

    optional(1) string optName;
    optional(2) int optInt;
    optional(3) ValStruct optValStruct;
    optional(4) RefStruct optRefStruct;
    optional(5) MyEnum optEnum;
    optional(6) MyClass optClass;
    optional(7) MyInterface* optProxy;
}

}
