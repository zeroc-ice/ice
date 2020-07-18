//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning:reserved-identifier]]

module ZeroC::Ice::Test::Enums
{

const byte ByteConst1 = 10;
const short ShortConst1 = 20;
const int IntConst1 = 30;
const long LongConst1 = 40;

const byte ByteConst2 = 226;
const short ShortConst2 = 32766;
const int IntConst2 = 2147483647;
const long LongConst2 = 2147483646;

enum ByteEnum
{
    benum1,
    benum2,
    benum3 = ByteConst1,
    benum4,
    benum5 = ShortConst1,
    benum6,
    benum7 = IntConst1,
    benum8,
    benum9 = LongConst1,
    benum10,
    benum11 = ByteConst2
}
sequence<ByteEnum> ByteEnumSeq;

enum ShortEnum
{
    senum1 = 3,
    senum2,
    senum3 = ByteConst1,
    senum4,
    senum5 = ShortConst1,
    senum6,
    senum7 = IntConst1,
    senum8,
    senum9 = LongConst1,
    senum10,
    senum11 = ShortConst2
}
sequence<ShortEnum> ShortEnumSeq;

enum IntEnum
{
    ienum1,
    ienum2,
    ienum3 = ByteConst1,
    ienum4,
    ienum5 = ShortConst1,
    ienum6,
    ienum7 = IntConst1,
    ienum8,
    ienum9 = LongConst1,
    ienum10,
    ienum11 = IntConst2,
    ienum12 = LongConst2
}
sequence<IntEnum> IntEnumSeq;

enum SimpleEnum
{
    red,
    green,
    blue
}
sequence<SimpleEnum> SimpleEnumSeq;

enum FLByteEnum : byte // Fixed-length
{
    benum1,
    benum2,
    benum3 = ByteConst1,
    benum4,
    benum5 = ShortConst1,
    benum6,
    benum7 = IntConst1,
    benum8,
    benum9 = LongConst1,
    benum10,
    benum11 = ByteConst2
}
sequence<FLByteEnum> FLByteEnumSeq;

enum FLShortEnum : short
{
    senum1 = -3,
    senum2,
    senum3 = ByteConst1,
    senum4,
    senum5 = ShortConst1,
    senum6,
    senum7 = IntConst1,
    senum8,
    senum9 = LongConst1,
    senum10,
    senum11 = ShortConst2
}
sequence<FLShortEnum> FLShortEnumSeq;

enum FLUShortEnum : ushort
{
    senum1 = 3,
    senum2,
    senum3 = ByteConst1,
    senum4,
    senum5 = ShortConst1,
    senum6,
    senum7 = IntConst1,
    senum8,
    senum9 = LongConst1,
    senum10,
    senum11 = ShortConst2
}
sequence<FLUShortEnum> FLUShortEnumSeq;

enum FLIntEnum : int
{
    ienum1 = -3,
    ienum2,
    ienum3 = ByteConst1,
    ienum4,
    ienum5 = ShortConst1,
    ienum6,
    ienum7 = IntConst1,
    ienum8,
    ienum9 = LongConst1,
    ienum10,
    ienum11 = IntConst2,
    ienum12 = LongConst2
}
sequence<FLIntEnum> FLIntEnumSeq;

enum FLUIntEnum : uint
{
    ienum1,
    ienum2,
    ienum3 = ByteConst1,
    ienum4,
    ienum5 = ShortConst1,
    ienum6,
    ienum7 = IntConst1,
    ienum8,
    ienum9 = LongConst1,
    ienum10,
    ienum11 = IntConst2,
    ienum12 = LongConst2
}
sequence<FLUIntEnum> FLUIntEnumSeq;

enum FLSimpleEnum : byte
{
    red,
    green,
    blue
}
sequence<FLSimpleEnum> FLSimpleEnumSeq;

[cs:attribute:System.Flags] unchecked enum MyFlags : uint
{
    E0 = 1,
    E1 = 2,
    E2 = 4,
    E3 = 8,
    E4 = 16,
    E5 = 32,
    E6 = 64,
    E7 = 128,
    E8 = 256,
    E9 = 512,
    E10 = 1024,
    E11 = 2048,
    E12 = 4096,
    E13 = 8192,
    E14 = 0x04000,
    E15 = 0x08000,
    E16 = 0x10000,
    E17 = 0x20000,
    E18 = 0x40000,
    E19 = 0x80000,
    // don't need them all for this test
    E29 = 0x20000000,
    E30 = 0x40000000,
    E31 = 0x80000000
}

sequence<MyFlags> MyFlagsSeq;

interface TestIntf
{
    ByteEnum opByte(ByteEnum b1, out ByteEnum b2);
    ShortEnum opShort(ShortEnum s1, out ShortEnum s2);
    IntEnum opInt(IntEnum i1, out IntEnum i2);
    SimpleEnum opSimple(SimpleEnum s1, out SimpleEnum s2);

    ByteEnumSeq opByteSeq(ByteEnumSeq b1, out ByteEnumSeq b2);
    ShortEnumSeq opShortSeq(ShortEnumSeq s1, out ShortEnumSeq s2);
    IntEnumSeq opIntSeq(IntEnumSeq i1, out IntEnumSeq i2);
    SimpleEnumSeq opSimpleSeq(SimpleEnumSeq s1, out SimpleEnumSeq s2);

    FLByteEnum opFLByte(FLByteEnum b1, out FLByteEnum b2);
    FLShortEnum opFLShort(FLShortEnum s1, out FLShortEnum s2);
    FLUShortEnum opFLUShort(FLUShortEnum s1, out FLUShortEnum s2);
    FLIntEnum opFLInt(FLIntEnum i1, out FLIntEnum i2);
    FLUIntEnum opFLUInt(FLUIntEnum i1, out FLUIntEnum i2);
    FLSimpleEnum opFLSimple(FLSimpleEnum s1, out FLSimpleEnum s2);

    FLByteEnumSeq opFLByteSeq(FLByteEnumSeq b1, out FLByteEnumSeq b2);
    FLShortEnumSeq opFLShortSeq(FLShortEnumSeq s1, out FLShortEnumSeq s2);
    FLUShortEnumSeq opFLUShortSeq(FLUShortEnumSeq s1, out FLUShortEnumSeq s2);
    FLIntEnumSeq opFLIntSeq(FLIntEnumSeq i1, out FLIntEnumSeq i2);
    FLUIntEnumSeq opFLUIntSeq(FLUIntEnumSeq i1, out FLUIntEnumSeq i2);
    FLSimpleEnumSeq opFLSimpleSeq(FLSimpleEnumSeq s1, out FLSimpleEnumSeq s2);

    tag(1) ByteEnum? opTaggedByte(tag(2) ByteEnum? b1, out tag(3) ByteEnum? b2);
    tag(1) FLByteEnum? opTaggedFLByte(tag(2) FLByteEnum? b1, out tag(3) FLByteEnum? b2);
    tag(1) ByteEnumSeq? opTaggedByteSeq(tag(2) ByteEnumSeq? b1, out tag(3) ByteEnumSeq? b2);
    tag(1) FLByteEnumSeq? opTaggedFLByteSeq(tag(2) FLByteEnumSeq? b1, out tag(3) FLByteEnumSeq? b2);
    tag(1) FLIntEnumSeq? opTaggedFLIntSeq(tag(2) FLIntEnumSeq? i1, out tag(3) FLIntEnumSeq? i2);

    MyFlags opMyFlags(MyFlags f1, out MyFlags f2);
    MyFlagsSeq opMyFlagsSeq(MyFlagsSeq f1, out MyFlagsSeq f2);
    tag(1) MyFlags? opTaggedMyFlags(tag(2) MyFlags? f1, out tag(3) MyFlags? f2);
    tag(1) MyFlagsSeq? opTaggedMyFlagsSeq(tag(2) MyFlagsSeq? f1, out tag(3) MyFlagsSeq? f2);

    void shutdown();
}

}
