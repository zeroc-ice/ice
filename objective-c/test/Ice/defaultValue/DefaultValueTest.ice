// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

["objc:prefix:TestDefaultValue"]
module Test
{

enum Color { red, green, blue }

struct Struct1
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 254;
    short s = 16000;
    int i = 3;
    long l = 4;
    float f = 5.0;
    double d = 6.0;
    string str = "foo bar";
    Color c = red;
    string noDefault;
}

["cpp:class"]
struct Struct2
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    float f = 5.0;
    double d = 6.0;
    string str = "foo bar";
    Color c = blue;
    string noDefault;
}

class Base
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    float f = 5.0;
    double d = 6.0;
    string str = "foo bar";
    string noDefault;
}

class Derived extends Base
{
    Color c = green;
}

exception BaseEx
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    float f = 5.0;
    double d = 6.0;
    string str = "foo bar";
    string noDefault;
}

exception DerivedEx extends BaseEx
{
    Color c = green;
}

sequence<byte> ByteSeq;
sequence<int> IntSeq;
dictionary<int, string> IntStringDict;

struct InnerStruct
{
    int a;
}

struct StructNoDefaults
{
    bool bo;
    byte b;
    short s;
    int i;
    long l;
    float f;
    double d;
    string str;
    Color c1;
    ByteSeq bs;
    IntSeq is;
    InnerStruct st;
    IntStringDict dict;
}

exception ExceptionNoDefaultsBase
{
    string str;
    Color c1;
    ByteSeq bs;
}

exception ExceptionNoDefaults extends ExceptionNoDefaultsBase
{
    InnerStruct st;
    IntStringDict dict;
}

class ClassNoDefaultsBase
{
    string str;
    Color c1;
    ByteSeq bs;
}

class ClassNoDefaults extends ClassNoDefaultsBase
{
    InnerStruct st;
    IntStringDict dict;
}

}
