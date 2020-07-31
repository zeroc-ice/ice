//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Compress
{

enum MyEnum
{
    enum1,
    enum2,
    enum3
}

struct MyStruct
{
    int i;
    int j;
}

class MyValue
{
    MyStruct s;
}

interface MyInterface
{
}

sequence<byte> ByteS;
sequence<bool> BoolS;
sequence<short> ShortS;
sequence<int> IntS;
sequence<varint> VarIntS;
sequence<long> LongS;
sequence<varlong> VarLongS;
sequence<ushort> UShortS;
sequence<uint> UIntS;
sequence<varuint> VarUIntS;
sequence<ulong> ULongS;
sequence<varulong> VarULongS;
sequence<float> FloatS;
sequence<double> DoubleS;
sequence<string> StringS;
sequence<MyEnum> MyEnumS;
sequence<MyStruct> MyStructS;
sequence<MyValue> MyValueS;
sequence<MyInterface*> MyInterfaceS;

interface TestIntf
{
    void shutdown();

    [compress:params,return] ByteS opByteS(ByteS p1, ByteS p2, out ByteS p3);
    [compress:params,return] BoolS opBoolS(BoolS p1, BoolS p2, out BoolS p3);
    [compress:params,return] LongS opShortIntLongS(
        ShortS p1,
        IntS p2,
        LongS p3,
        out ShortS p4,
        out IntS p5,
        out LongS p6);
    [compress:params,return] ULongS opUShortUIntULongS(
        UShortS p1,
        UIntS p2,
        ULongS p3,
        out UShortS p4,
        out UIntS p5,
        out ULongS p6);
    [compress:params,return] VarLongS opVarIntVarLongS(VarIntS p1, VarLongS p2, out VarIntS p3, out VarLongS p4);
    [compress:params,return] VarULongS opVarUIntVarULongS(
        VarUIntS p1,
        VarULongS p2,
        out VarUIntS p3,
        out VarULongS p4);
    [compress:params,return] DoubleS opFloatDoubleS(FloatS p1, DoubleS p2, out FloatS p3, out DoubleS p4);
    [compress:params,return] StringS opStringS(StringS p1, StringS p2, out StringS p3);
    [compress:params,return] MyStructS opMyStructS(MyStructS p1, MyStructS p2, out MyStructS p3);
    [compress:params,return] MyValueS opMyValueS(MyValueS p1, MyValueS p2, out MyValueS p3);
    [compress:params,return] MyInterfaceS opMyInterfaceS(MyInterfaceS p1, MyInterfaceS p2, out MyInterfaceS p3);
    [compress:params,return] MyEnumS opMyEnumS(MyEnumS p1, MyEnumS p2, out MyEnumS p3);
}

}
