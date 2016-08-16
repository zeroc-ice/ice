// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/Current.ice>

module Test
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
};

class MyClass;

struct AnotherStruct
{
    string s;
};

struct Structure
{
    MyClass* p;
    MyEnum e;
    AnotherStruct s;
};

sequence<byte> ByteS;
sequence<bool> BoolS;
sequence<short> ShortS;
sequence<int> IntS;
sequence<long> LongS;
sequence<float> FloatS;
sequence<double> DoubleS;
sequence<string> StringS;
sequence<MyEnum> MyEnumS;
sequence<MyClass*> MyClassS;

sequence<ByteS> ByteSS;
sequence<BoolS> BoolSS;
sequence<ShortS> ShortSS;
sequence<IntS> IntSS;
sequence<LongS> LongSS;
sequence<FloatS> FloatSS;
sequence<DoubleS> DoubleSS;
sequence<StringS> StringSS;
sequence<MyEnumS> MyEnumSS;
sequence<MyClassS> MyClassSS;

sequence<StringSS> StringSSS;

struct MyStruct
{
    int i;
    int j;
};

dictionary<byte, bool> ByteBoolD;
dictionary<short, int> ShortIntD;
dictionary<long, float> LongFloatD;
dictionary<string, string> StringStringD;
dictionary<string, MyEnum> StringMyEnumD;
dictionary<MyEnum, string> MyEnumStringD;
dictionary<MyStruct, MyEnum> MyStructMyEnumD;

sequence<ByteBoolD> ByteBoolDS;
sequence<ShortIntD> ShortIntDS;
sequence<LongFloatD> LongFloatDS;
sequence<StringStringD> StringStringDS;
sequence<StringMyEnumD> StringMyEnumDS;
sequence<MyEnumStringD> MyEnumStringDS;
sequence<MyStructMyEnumD> MyStructMyEnumDS;

dictionary<byte, ByteS> ByteByteSD;
dictionary<bool, BoolS> BoolBoolSD;
dictionary<short, ShortS> ShortShortSD;
dictionary<int, IntS> IntIntSD;
dictionary<long, LongS> LongLongSD;
dictionary<string, FloatS> StringFloatSD;
dictionary<string, DoubleS> StringDoubleSD;
dictionary<string, StringS> StringStringSD;
dictionary<MyEnum, MyEnumS> MyEnumMyEnumSD;

exception SomeException {};

class MyClass
{
    void shutdown();

    void opVoid();

    byte opByte(byte p1, byte p2,
                out byte p3);

    bool opBool(bool p1, bool p2,
                out bool p3);

    long opShortIntLong(short p1, int p2, long p3,
                        out short p4, out int p5, out long p6);

    double opFloatDouble(float p1, double p2,
                         out float p3, out double p4);

    string opString(string p1, string p2,
                    out string p3);

    MyEnum opMyEnum(MyEnum p1, out MyEnum p2);

    MyClass* opMyClass(MyClass* p1, out MyClass* p2, out MyClass* p3);

    Structure opStruct(Structure p1, Structure p2,
                       out Structure p3);

    ByteS opByteS(ByteS p1, ByteS p2,
                  out ByteS p3);

    BoolS opBoolS(BoolS p1, BoolS p2,
                  out BoolS p3);

    LongS opShortIntLongS(Test::ShortS p1, IntS p2, LongS p3,
                          out ::Test::ShortS p4, out IntS p5, out LongS p6);

    DoubleS opFloatDoubleS(FloatS p1, DoubleS p2,
                           out FloatS p3, out DoubleS p4);

    StringS opStringS(StringS p1, StringS p2,
                      out StringS p3);

    ByteSS opByteSS(ByteSS p1, ByteSS p2,
                    out ByteSS p3);

    BoolSS opBoolSS(BoolSS p1, BoolSS p2,
                    out BoolSS p3);

    LongSS opShortIntLongSS(ShortSS p1, IntSS p2, LongSS p3,
                            out ShortSS p4, out IntSS p5, out LongSS p6);


    DoubleSS opFloatDoubleSS(FloatSS p1, DoubleSS p2,
                             out FloatSS p3, out DoubleSS p4);

    StringSS opStringSS(StringSS p1, StringSS p2,
                        out StringSS p3);

    StringSSS opStringSSS(StringSSS p1, StringSSS p2,
                          out StringSSS p3);

    ByteBoolD opByteBoolD(ByteBoolD p1, ByteBoolD p2,
                          out ByteBoolD p3);

    ShortIntD opShortIntD(ShortIntD p1, ShortIntD p2,
                          out ShortIntD p3);

    LongFloatD opLongFloatD(LongFloatD p1, LongFloatD p2,
                            out LongFloatD p3);

    StringStringD opStringStringD(StringStringD p1, StringStringD p2,
                                  out StringStringD p3);

    StringMyEnumD opStringMyEnumD(StringMyEnumD p1, StringMyEnumD p2,
                                  out StringMyEnumD p3);

    MyEnumStringD opMyEnumStringD(MyEnumStringD p1, MyEnumStringD p2,
                                  out MyEnumStringD p3);

    MyStructMyEnumD opMyStructMyEnumD(MyStructMyEnumD p1, MyStructMyEnumD p2,
                                      out MyStructMyEnumD p3);

    ByteBoolDS opByteBoolDS(ByteBoolDS p1, ByteBoolDS p2,
                            out ByteBoolDS p3);

    ShortIntDS opShortIntDS(ShortIntDS p1, ShortIntDS p2,
                            out ShortIntDS p3);

    LongFloatDS opLongFloatDS(LongFloatDS p1, LongFloatDS p2,
                              out LongFloatDS p3);

    StringStringDS opStringStringDS(StringStringDS p1, StringStringDS p2,
                                    out StringStringDS p3);

    StringMyEnumDS opStringMyEnumDS(StringMyEnumDS p1, StringMyEnumDS p2,
                                    out StringMyEnumDS p3);

    MyEnumStringDS opMyEnumStringDS(MyEnumStringDS p1, MyEnumStringDS p2,
                                    out MyEnumStringDS p3);

    MyStructMyEnumDS opMyStructMyEnumDS(MyStructMyEnumDS p1, MyStructMyEnumDS p2,
                                        out MyStructMyEnumDS p3);

    ByteByteSD opByteByteSD(ByteByteSD p1, ByteByteSD p2,
                            out ByteByteSD p3);

    BoolBoolSD opBoolBoolSD(BoolBoolSD p1, BoolBoolSD p2,
                            out BoolBoolSD p3);

    ShortShortSD opShortShortSD(ShortShortSD p1, ShortShortSD p2,
                                out ShortShortSD p3);

    IntIntSD opIntIntSD(IntIntSD p1, IntIntSD p2,
                        out IntIntSD p3);

    LongLongSD opLongLongSD(LongLongSD p1, LongLongSD p2,
                            out LongLongSD p3);

    StringFloatSD opStringFloatSD(StringFloatSD p1, StringFloatSD p2,
                                  out StringFloatSD p3);

    StringDoubleSD opStringDoubleSD(StringDoubleSD p1, StringDoubleSD p2,
                                    out StringDoubleSD p3);

    StringStringSD opStringStringSD(StringStringSD p1, StringStringSD p2,
                                    out StringStringSD p3);

    MyEnumMyEnumSD opMyEnumMyEnumSD(MyEnumMyEnumSD p1, MyEnumMyEnumSD p2,
                                    out MyEnumMyEnumSD p3);

    IntS opIntS(IntS s);

    void opByteSOneway(ByteS s);

    int opByteSOnewayCallCount();

    Ice::Context opContext();

    void opDoubleMarshaling(double p1, DoubleS p2);

    idempotent void opIdempotent();

    ["nonmutating"] idempotent void opNonmutating();

    byte opByte1(byte opByte1);
    short opShort1(short opShort1);
    int opInt1(int opInt1);
    long opLong1(long opLong1);
    float opFloat1(float opFloat1);
    double opDouble1(double opDouble1);
    string opString1(string opString1);
    StringS opStringS1(StringS opStringS1);
    ByteBoolD opByteBoolD1(ByteBoolD opByteBoolD1);
    StringS opStringS2(StringS stringS);
    ByteBoolD opByteBoolD2(ByteBoolD byteBoolD);
};

struct MyStruct1
{
    string tesT; // Same name as the enclosing module
    MyClass myClass; // Same name as an already defined class
    string myStruct1; // Same name as the enclosing struct
};

class MyClass1
{
    string tesT; // Same name as the enclosing module
    MyClass myClass; // Same name as an already defined class
    string myClass1; // Same name as the enclosing class
};

class MyDerivedClass extends MyClass
{
    void opDerived();
    MyClass1 opMyClass1(MyClass1 c);
    MyStruct1 opMyStruct1(MyStruct1 c);
};

};

