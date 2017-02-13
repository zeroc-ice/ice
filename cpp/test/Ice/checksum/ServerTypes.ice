// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["suppress-warning:deprecated"]] // For classes with operations

module Test
{

//
// TEST: Same
//
const int IntConst1 = 100;

//
// TEST: Value changed
//
const int IntConst2 = 1000;

//
// TEST: Type changed
//
const short IntConst3 = 100;

//
// TEST: Same
//
enum Enum1 { Enum11, Enum12, Enum13 };

//
// TEST: Add enumerator
//
enum Enum2 { Enum21, Enum22, Enum23, Enum24 };

//
// TEST: Remove enumerator
//
enum Enum3 { Enum32, Enum33 };

//
// TEST: Enum with explicit values.
//
enum EnumExplicit0 { EnumExplicit01 = 1, EnumExplicit02 = 2, EnumExplicit03 = 3 };

//
// TEST: Enum with same explicit values, different order.
//
enum EnumExplicit1 { EnumExplicit11 = 1, EnumExplicit13 = 3, EnumExplicit12 = 2 };

//
// TEST: Enum with different explicit values.
//
enum EnumExplicit2 { EnumExplicit21 = 1, EnumExplicit22 = 3, EnumExplicit23 };

//
// TEST: Enum with explicit values, removed enumerator.
//
enum EnumExplicit3 { EnumExplicit31 = 1, EnumExplicit32 = 2};

//
// TEST: Change to a different type
//
class Enum4 {};

//
// TEST: Same
//
sequence<int> Sequence1;

//
// TEST: Change sequence type
//
sequence<short> Sequence2;

//
// TEST: Change to a different type
//
class Sequence3 {};

//
// TEST: Same
//
dictionary<string, int> Dictionary1;

//
// TEST: Change key type
//
dictionary<long, int> Dictionary2;

//
// TEST: Change value type
//
dictionary<string, bool> Dictionary3;

//
// TEST: Change to a different type
//
class Dictionary4 {};

//
// TEST: Same
//
struct Struct1
{
    string str;
    bool b;
};

//
// TEST: Add member
//
struct Struct2
{
    string str;
    bool b;
    float f;
};

//
// TEST: Change member type
//
struct Struct3
{
    string str;
    double b;
};

//
// TEST: Remove member
//
struct Struct4
{
    bool b;
};

//
// TEST: Change to a different type
//
class Struct5 {};

//
// TEST: Same
//
interface Interface1
{
};

//
// TEST: Change interface to class
//
class Interface2
{
};

//
// TEST: Add base interface
//
interface Interface3 extends Interface1
{
};

//
// TEST: Add operation
//
interface Interface4
{
    void opInterface4();
};

//
// TEST: Same
//
class EmptyClass1
{
};

//
// TEST: Add data member
//
class EmptyClass2
{
    double d;
};

//
// TEST: Add operation
//
class EmptyClass3
{
    void newOp();
};

//
// TEST: Add base class
//
class EmptyClass4 extends EmptyClass1
{
};

//
// TEST: Add interface
//
class EmptyClass5 implements Interface1
{
};

//
// TEST: Same
//
class SimpleClass1
{
    string str;
    float f;
};

//
// TEST: Add operation
//
class SimpleClass2
{
    string str;
    float f;
    void newOp();
};

//
// TEST: Rename member
//
class SimpleClass3
{
    string str;
    float g;
};

//
// TEST: Add member
//
class SimpleClass4
{
    string str;
    float f;
    bool b;
};

//
// TEST: Remove member
//
class SimpleClass5
{
    string str;
};

//
// TEST: Reorder members
//
class SimpleClass6
{
    float f;
    string str;
};

//
// TEST: Change member type
//
class SimpleClass7
{
    string str;
    double f;
};

//
// TEST: Same
//
exception Exception1
{
    string str;
    bool b;
};

//
// TEST: Add member
//
exception Exception2
{
    string str;
    bool b;
    float f;
};

//
// TEST: Change member type
//
exception Exception3
{
    string str;
    double b;
};

//
// TEST: Remove member
//
exception Exception4
{
    bool b;
};

//
// TEST: Add base exception
//
exception Exception5 extends Exception1
{
};

//
// TEST: Change to a different type
//
class Exception6 {};

//
// TEST: Exception with optional members.
//
exception OptionalEx0
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
};

//
// TEST: Exception with optional members, different order, same tags.
//
exception OptionalEx1
{
    string firstName;
    optional(2) string emailAddress;
    optional(1) string secondName;
};

//
// TEST: Exception with different optional members.
//
exception OptionalEx2
{
    string firstName;
    optional(1) string secondName;
    string emailAddress;
};

//
// TEST: Exception with different optional members.
//
exception OptionalEx3
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
    optional(3) string phoneNumber;
};

//
// TEST: Exception with optional members using different tags.
//
exception OptionalEx4
{
    string firstName;
    optional(2) string secondName;
    optional(1) string emailAddress;
};

//
// TEST: Same
//
interface BaseInterface1
{
    void baseOp1();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Change return type
//
interface BaseInterface2
{
    int baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Add parameter
//
interface BaseInterface3
{
    void baseOp(Object o);
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Add exception
//
interface BaseInterface4
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1, Exception2;
};

//
// TEST: Change out parameter to in parameter
//
interface BaseInterface5
{
    void baseOp();
    void baseOp2(int i, string s) throws Exception1;
};

//
// TEST: Remove parameter
//
interface BaseInterface6
{
    void baseOp();
    void baseOp2(out string s) throws Exception1;
};

//
// TEST: Remove exception
//
interface BaseInterface7
{
    void baseOp();
    void baseOp2(int i, out string s);
};

//
// TEST: Remove operation
//
interface BaseInterface8
{
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Add base interface
//
interface BaseInterface9 extends Interface1
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Class with compact id
//
class Compact1(1)
{
    int id;
};

//
// TEST: Derived from class with compact id
//
class Derived1 extends Compact1
{
};

//
// TEST: Same class names but different compact id
//
class Compact2(3)
{
    int id;
};

//
// TEST: Class with optional members.
//
class Optional0
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
};

//
// TEST: Class with optional members, different order, same tags.
//
class Optional1
{
    string firstName;
    optional(2) string emailAddress;
    optional(1) string secondName;
};

//
// TEST: Class with different optional members.
//
class Optional2
{
    string firstName;
    optional(1) string secondName;
    string emailAddress;
};

//
// TEST: Class with different optional members.
//
class Optional3
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
    optional(3) string phoneNumber;
};

//
// TEST: Class with optional members using different tags.
//
class Optional4
{
    string firstName;
    optional(2) string secondName;
    optional(1) string emailAddress;
};

//
// TEST: Interface with optional parameters.
//
interface OptionalParameters0
{
    void op1(string firstName, optional(1) string secondName,
             optional(2) string emailAddress);
};

//
// TEST: Interface with optional parameters, different order.
//
interface OptionalParameters1
{
    void op1(string firstName, optional(2) string emailAddress,
             optional(1) string secondName);
};

//
// TEST: Interface with optional parameters, different tags.
//
interface OptionalParameters2
{
    void op1(string firstName, optional(2) string emailAddress,
             optional(1) string secondName);
};

//
// TEST: Interface with different optional parameters.
//
interface OptionalParameters3
{
    void op1(string firstName, string emailAddress,
             optional(1) string secondName);
};

//
// TEST: Interface with optional return type.
//
interface OptionalReturn0
{
    optional(1) int op();
};

//
// TEST: Interface that changes optional return type.
//
interface OptionalReturn2
{
    int op();
};

//
// TEST: Local
//
local enum LocalEnum { LocalEnum1, LocalEnum2, LocalEnum3 };

//
// TEST: Local
//
local sequence<string> LocalSequence;

//
// TEST: Local
//
local dictionary<string, string> LocalDictionary;

//
// TEST: Local
//
local struct LocalStruct
{
    string str;
};

//
// TEST: Local
//
local class LocalClass
{
};

};
