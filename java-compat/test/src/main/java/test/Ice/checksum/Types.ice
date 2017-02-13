// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["java:package:test.Ice.checksum"]]
module Test
{

//
// TEST: Same
//
const int IntConst1 = 100;

//
// TEST: Value changed
//
const int IntConst2 = 100;

//
// TEST: Type changed
//
const int IntConst3 = 100;

//
// TEST: Same
//
enum Enum1 { Enum11, Enum12, Enum13 };

//
// TEST: Add enumerator
//
enum Enum2 { Enum21, Enum22, Enum23 };

//
// TEST: Remove enumerator
//
enum Enum3 { Enum31, Enum32, Enum33 };

//
// TEST: Change to a different type
//
enum Enum4 { Enum41, Enum42, Enum43 };

//
// TEST: Enum with explicit values.
//
enum EnumExplicit0 { EnumExplicit01 = 1, EnumExplicit02 = 2, EnumExplicit03 = 3 };

//
// TEST: Enum with same explicit values, different order.
//
enum EnumExplicit1 { EnumExplicit11 = 1, EnumExplicit12 = 2, EnumExplicit13 = 3 };

//
// TEST: Enum with different explicit values.
//
enum EnumExplicit2 { EnumExplicit21 = 1, EnumExplicit22 = 2, EnumExplicit23 = 3};

//
// TEST: Enum with explicit values, removed enumerator.
//
enum EnumExplicit3 { EnumExplicit31 = 1, EnumExplicit32 = 2, EnumExplicit33 = 3};

//
// TEST: Same
//
sequence<int> Sequence1;

//
// TEST: Change sequence type
//
sequence<int> Sequence2;

//
// TEST: Change to a different type
//
sequence<int> Sequence3;

//
// TEST: Same
//
dictionary<string, int> Dictionary1;

//
// TEST: Change key type
//
dictionary<string, int> Dictionary2;

//
// TEST: Change value type
//
dictionary<string, int> Dictionary3;

//
// TEST: Change to a different type
//
dictionary<string, int> Dictionary4;

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
};

//
// TEST: Change member type
//
struct Struct3
{
    string str;
    bool b;
};

//
// TEST: Remove member
//
struct Struct4
{
    string str;
    bool b;
};

//
// TEST: Change to a different type
//
struct Struct5
{
    string str;
    bool b;
};

//
// TEST: Same
//
interface Interface1
{
};

//
// TEST: Change interface to class
//
interface Interface2
{
};

//
// TEST: Add base interface
//
interface Interface3
{
};

//
// TEST: Add operation
//
interface Interface4
{
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
};

//
// TEST: Add operation
//
class EmptyClass3
{
};

//
// TEST: Add base class
//
class EmptyClass4
{
};

//
// TEST: Add interface
//
class EmptyClass5
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
};

//
// TEST: Rename member
//
class SimpleClass3
{
    string str;
    float f;
};

//
// TEST: Add member
//
class SimpleClass4
{
    string str;
    float f;
};

//
// TEST: Remove member
//
class SimpleClass5
{
    string str;
    float f;
};

//
// TEST: Reorder members
//
class SimpleClass6
{
    string str;
    float f;
};

//
// TEST: Change member type
//
class SimpleClass7
{
    string str;
    float f;
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
};

//
// TEST: Change member type
//
exception Exception3
{
    string str;
    bool b;
};

//
// TEST: Remove member
//
exception Exception4
{
    string str;
    bool b;
};

//
// TEST: Add base exception
//
exception Exception5
{
};

//
// TEST: Change to a different type
//
exception Exception6
{
    string str;
    bool b;
};

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
    optional(1) string secondName;
    optional(2) string emailAddress;
};

//
// TEST: Exception with different optional members.
//
exception OptionalEx2
{
    string firstName;
    string secondName;
    optional(1) string emailAddress;
};

//
// TEST: Exception with different optional members.
//
exception OptionalEx3
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
};

//
// TEST: Exception with optional members using different tags.
//
exception OptionalEx4
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
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
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Add parameter
//
interface BaseInterface3
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Add exception
//
interface BaseInterface4
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Change out parameter to in parameter
//
interface BaseInterface5
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Remove parameter
//
interface BaseInterface6
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Remove exception
//
interface BaseInterface7
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Remove operation
//
interface BaseInterface8
{
    void baseOp();
    void baseOp2(int i, out string s) throws Exception1;
};

//
// TEST: Add base interface
//
interface BaseInterface9
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
class Compact2(2)
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
    optional(1) string secondName;
    optional(2) string emailAddress;
};

//
// TEST: Class with different optional members.
//
class Optional2
{
    string firstName;
    string secondName;
    optional(1) string emailAddress;
};

//
// TEST: Class with different optional members.
//
class Optional3
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
};

//
// TEST: Class with optional members using different tags.
//
class Optional4
{
    string firstName;
    optional(1) string secondName;
    optional(2) string emailAddress;
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
    void op1(string firstName, optional(1) string secondName,
             optional(2) string emailAddress);
};

//
// TEST: Interface with optional parameters, different tags.
//
interface OptionalParameters2
{
    void op1(string firstName, optional(1) string emailAddress,
             optional(2) string secondName);
};

//
// TEST: Interface with different optional parameters.
//
interface OptionalParameters3
{
    void op1(string firstName, optional(1) string emailAddress,
             string secondName);
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
    optional(1) int op();
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
