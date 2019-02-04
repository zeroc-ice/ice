//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["suppress-warning:deprecated"]] // For enumerator references

module Test
{

enum Color { red, green, blue }

module Nested
{

enum Color { red, green, blue }

}

struct Struct1
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 254;
    short s = 16000;
    int i = 3;
    long l = 4;
    float f = 5.1;
    double d = 6.2;
    string str = "foo \\ \"bar\n \r\n\t\v\f\a\b\?";
    Color c1 = ::Test::Color::red;
    Color c2 = Test::green;
    Color c3 = blue;
    Nested::Color nc1 = Test::Nested::Color::red;
    Nested::Color nc2 = Nested::green;
    Nested::Color nc3 = blue;
    string noDefault;
    int zeroI = 0;
    long zeroL = 0;
    float zeroF = 0;
    float zeroDotF = 0.0;
    double zeroD = 0;
    double zeroDotD = 0;
}

const bool ConstBool = true;
const byte ConstByte = 254;
const short ConstShort = 16000;
const int ConstInt = 3;
const long ConstLong = 4;
const float ConstFloat = 5.1;
const double ConstDouble = 6.2;
const string ConstString = "foo \\ \"bar\n \r\n\t\v\f\a\b\?";
const Color ConstColor1 = ::Test::Color::red;
const Color ConstColor2 = Test::green;
const Color ConstColor3 = blue;
const Nested::Color ConstNestedColor1 = Test::Nested::Color::red;
const Nested::Color ConstNestedColor2 = Test::Nested::green;
const Nested::Color ConstNestedColor3 = blue;
const int ConstZeroI = 0;
const long ConstZeroL = 0;
const float ConstZeroF = 0;
const float ConstZeroDotF = 0.0;
const double ConstZeroD = 0;
const double ConstZeroDotD = 0;

struct Struct2
{
    bool boolTrue = ConstBool;
    byte b = ConstByte;
    short s = ConstShort;
    int i = ConstInt;
    long l = ConstLong;
    float f = ConstFloat;
    double d = ConstDouble;
    string str = ConstString;
    Color c1 = ConstColor1;
    Color c2 = ConstColor2;
    Color c3 = ConstColor3;
    Nested::Color nc1 = ConstNestedColor1;
    Nested::Color nc2 = ConstNestedColor2;
    Nested::Color nc3 = ConstNestedColor3;
    int zeroI = ConstZeroI;
    long zeroL = ConstZeroL;
    float zeroF = ConstZeroF;
    float zeroDotF = ConstZeroDotF;
    double zeroD = ConstZeroD;
    double zeroDotD = ConstZeroDotD;
}

["cpp:class"]
struct Struct3
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    float f = 5.1;
    double d = 6.2;
    string str = "foo \\ \"bar\n \r\n\t\v\f\a\b\?";
    Color c1 = ::Test::Color::red;
    Color c2 = Test::green;
    Color c3 = blue;
    Nested::Color nc1 = ::Test::Nested::Color::red;
    Nested::Color nc2 = Nested::green;
    Nested::Color nc3 = blue;
    string noDefault;
    int zeroI = 0;
    long zeroL = 0;
    float zeroF = 0;
    float zeroDotF = 0.0;
    double zeroD = 0;
    double zeroDotD = 0;
}

class Base
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    float f = 5.1;
    double d = 6.2;
    string str = "foo \\ \"bar\n \r\n\t\v\f\a\b\?";
    string noDefault;
    int zeroI = 0;
    long zeroL = 0;
    float zeroF = 0;
    float zeroDotF = 0.0;
    double zeroD = 0;
    double zeroDotD = 0;
}

class Derived extends Base
{
    Color c1 = ::Test::Color::red;
    Color c2 = Test::green;
    Color c3 = blue;
    Nested::Color nc1 = ::Test::Nested::Color::red;
    Nested::Color nc2 = Nested::green;
    Nested::Color nc3 = blue;
}

exception BaseEx
{
    bool boolFalse = false;
    bool boolTrue = true;
    byte b = 1;
    short s = 2;
    int i = 3;
    long l = 4;
    float f = 5.1;
    double d = 6.2;
    string str = "foo \\ \"bar\n \r\n\t\v\f\a\b\?";
    string noDefault;
    int zeroI = 0;
    long zeroL = 0;
    float zeroF = 0;
    float zeroDotF = 0.0;
    double zeroD = 0;
    double zeroDotD = 0;
}

exception DerivedEx extends BaseEx
{
    Color c1 = ConstColor1;
    Color c2 = ConstColor2;
    Color c3 = ConstColor3;
    Nested::Color nc1 = ConstNestedColor1;
    Nested::Color nc2 = ConstNestedColor2;
    Nested::Color nc3 = ConstNestedColor3;
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
