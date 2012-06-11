// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:include:list"]]

module Test
{

class OneOptional
{
    optional(1) int a;
};

enum MyEnum
{
    MyEnumMember
};

struct FixedStruct
{
    int m; 
};

struct VarStruct
{
    string m;
};

["cpp:class"] struct ClassVarStruct
{
    int a;
};

sequence<byte> ByteSeq;
sequence<bool> BoolSeq;
sequence<short> ShortSeq;
sequence<string> StringSeq;
sequence<MyEnum> MyEnumSeq;
sequence<FixedStruct> FixedStructSeq;
sequence<VarStruct> VarStructSeq;
sequence<OneOptional> OneOptionalSeq;
sequence<OneOptional*> OneOptionalPrxSeq;

dictionary<int, int> IntIntDict; 
dictionary<string, int> StringIntDict; 
dictionary<int, MyEnum> IntEnumDict;
dictionary<int, FixedStruct> IntFixedStructDict;
dictionary<int, VarStruct> IntVarStructDict;
dictionary<int, OneOptional> IntOneOptionalDict;
dictionary<int, OneOptional*> IntOneOptionalPrxDict;

class MultiOptional
{
    optional(1) byte a;
    optional(2) bool  b;
    optional(3) short c;
    optional(4) int d;
    optional(5) long e;
    optional(6) float f;
    optional(7) double g;
    optional(8) string h;
    optional(9) MyEnum i;
    optional(10) MultiOptional* j;
    optional(11) MultiOptional k;
    optional(12) ByteSeq bs;
    optional(13) StringSeq ss;
    optional(14) IntIntDict iid;
    optional(15) StringIntDict sid;
    optional(16) FixedStruct fs;
    optional(17) VarStruct vs;

    optional(18) ShortSeq shs;
    optional(19) MyEnumSeq es;
    optional(20) FixedStructSeq fss;
    optional(21) VarStructSeq vss;
    optional(22) OneOptionalSeq oos;
    optional(23) OneOptionalPrxSeq oops;

    optional(24) IntEnumDict ied;
    optional(25) IntFixedStructDict ifsd;
    optional(26) IntVarStructDict ivsd;
    optional(27) IntOneOptionalDict iood;
    optional(28) IntOneOptionalPrxDict ioopd;

    optional(29) BoolSeq bos;
};

class A
{
    int requiredA;
    optional(1) int ma;
    optional(50) int mb;
    optional(500) int mc;
};

["preserve-slice"]
class B extends A
{
    int requiredB;
    optional(10) int md;
};

class C extends B
{
    string ss;
    optional(890) string ms;
};

class WD
{
    optional(1) int a = 5;
    optional(2) string s = "test";
};

exception OptionalException
{
    optional(1) int a = 5;
    optional(2) string b;
    optional(50) OneOptional o;
};

class OptionalWithCustom
{
    ["cpp:type:std::list< ::Ice::Byte>"] optional(1) ByteSeq bs;
    optional(2) ClassVarStruct s;
};

class Initial
{
    void shutdown();

    Object pingPong(Object o);

    void opOptionalException(optional(1) int a, optional(2) string b, optional(3) OneOptional o)
        throws OptionalException;

    optional(1) byte opByte(optional(2) byte p1, out optional(3) byte p3);

    optional(1) long opLong(optional(2) long p1, out optional(3) long p3);

    optional(1) string opString(optional(2) string p1, out optional(3) string p3);

    optional(1) OneOptional opOneOptional(optional(2) OneOptional p1, out optional(3) OneOptional p3);

    optional(1) OneOptional* opOneOptionalProxy(optional(2) OneOptional* p1, out optional(3) OneOptional* p3);

    // Custom mapping operations
    ["cpp:array"] optional(1) ByteSeq opByteSeq(["cpp:array"] optional(2) ByteSeq p1, 
                                                out ["cpp:array"] optional(3) ByteSeq p3);

    ["cpp:array"] optional(1) ShortSeq opShortSeq(["cpp:array"] optional(2) ShortSeq p1, 
                                                  out ["cpp:array"] optional(3) ShortSeq p3);

    ["cpp:range:array"] optional(1) BoolSeq opBoolSeq(["cpp:range:array"] optional(2) BoolSeq p1, 
                                                      out ["cpp:range:array"] optional(3) BoolSeq p3);

    ["cpp:range"] optional(1) StringSeq opStringSeq(["cpp:range"] optional(2) StringSeq p1, 
                                                    out ["cpp:range"] optional(3) StringSeq p3);

    ["cpp:array"] optional(1) FixedStructSeq opFixedStructSeq(["cpp:array"] optional(2) FixedStructSeq p1, 
                                                              out ["cpp:array"] optional(3) FixedStructSeq p3);

    ["cpp:range"] optional(1) VarStructSeq opVarStructSeq(["cpp:range"] optional(2) VarStructSeq p1, 
                                                          out ["cpp:range"] optional(3) VarStructSeq p3);
};

};
