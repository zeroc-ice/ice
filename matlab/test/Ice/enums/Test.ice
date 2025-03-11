// Copyright (c) ZeroC, Inc.

#pragma once

module Test
{
    const byte ByteConst1 = 10;
    const short ShortConst1 = 20;
    const int IntConst1 = 30;
    const long LongConst1 = 40;

    const byte ByteConst2 = 126;
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

        void shutdown();
    }
}
