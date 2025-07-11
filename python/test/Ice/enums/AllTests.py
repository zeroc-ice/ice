# Copyright (c) ZeroC, Inc.

import sys

import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper, communicator):
    proxy = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint()}")
    test(proxy)

    sys.stdout.write("testing enum values... ")
    sys.stdout.flush()

    test(Test.ByteEnum.benum1.value == 0)
    test(Test.ByteEnum.benum2.value == 1)
    test(Test.ByteEnum.benum3.value == Test.ByteConst1)
    test(Test.ByteEnum.benum4.value == Test.ByteConst1 + 1)
    test(Test.ByteEnum.benum5.value == Test.ShortConst1)
    test(Test.ByteEnum.benum6.value == Test.ShortConst1 + 1)
    test(Test.ByteEnum.benum7.value == Test.IntConst1)
    test(Test.ByteEnum.benum8.value == Test.IntConst1 + 1)
    test(Test.ByteEnum.benum9.value == Test.LongConst1)
    test(Test.ByteEnum.benum10.value == Test.LongConst1 + 1)
    test(Test.ByteEnum.benum11.value == Test.ByteConst2)

    test(Test.ByteEnum(0) == Test.ByteEnum.benum1)
    test(Test.ByteEnum(1) == Test.ByteEnum.benum2)
    test(Test.ByteEnum(Test.ByteConst1) == Test.ByteEnum.benum3)
    test(Test.ByteEnum(Test.ByteConst1 + 1) == Test.ByteEnum.benum4)
    test(Test.ByteEnum(Test.ShortConst1) == Test.ByteEnum.benum5)
    test(Test.ByteEnum(Test.ShortConst1 + 1) == Test.ByteEnum.benum6)
    test(Test.ByteEnum(Test.IntConst1) == Test.ByteEnum.benum7)
    test(Test.ByteEnum(Test.IntConst1 + 1) == Test.ByteEnum.benum8)
    test(Test.ByteEnum(Test.LongConst1) == Test.ByteEnum.benum9)
    test(Test.ByteEnum(Test.LongConst1 + 1) == Test.ByteEnum.benum10)
    test(Test.ByteEnum(Test.ByteConst2) == Test.ByteEnum.benum11)

    test(Test.ShortEnum.senum1.value == 3)
    test(Test.ShortEnum.senum2.value == 4)
    test(Test.ShortEnum.senum3.value == Test.ByteConst1)
    test(Test.ShortEnum.senum4.value == Test.ByteConst1 + 1)
    test(Test.ShortEnum.senum5.value == Test.ShortConst1)
    test(Test.ShortEnum.senum6.value == Test.ShortConst1 + 1)
    test(Test.ShortEnum.senum7.value == Test.IntConst1)
    test(Test.ShortEnum.senum8.value == Test.IntConst1 + 1)
    test(Test.ShortEnum.senum9.value == Test.LongConst1)
    test(Test.ShortEnum.senum10.value == Test.LongConst1 + 1)
    test(Test.ShortEnum.senum11.value == Test.ShortConst2)

    test(Test.ShortEnum(3) == Test.ShortEnum.senum1)
    test(Test.ShortEnum(4) == Test.ShortEnum.senum2)
    test(Test.ShortEnum(Test.ByteConst1) == Test.ShortEnum.senum3)
    test(Test.ShortEnum(Test.ByteConst1 + 1) == Test.ShortEnum.senum4)
    test(Test.ShortEnum(Test.ShortConst1) == Test.ShortEnum.senum5)
    test(Test.ShortEnum(Test.ShortConst1 + 1) == Test.ShortEnum.senum6)
    test(Test.ShortEnum(Test.IntConst1) == Test.ShortEnum.senum7)
    test(Test.ShortEnum(Test.IntConst1 + 1) == Test.ShortEnum.senum8)
    test(Test.ShortEnum(Test.LongConst1) == Test.ShortEnum.senum9)
    test(Test.ShortEnum(Test.LongConst1 + 1) == Test.ShortEnum.senum10)
    test(Test.ShortEnum(Test.ShortConst2) == Test.ShortEnum.senum11)

    test(Test.IntEnum.ienum1.value == 0)
    test(Test.IntEnum.ienum2.value == 1)
    test(Test.IntEnum.ienum3.value == Test.ByteConst1)
    test(Test.IntEnum.ienum4.value == Test.ByteConst1 + 1)
    test(Test.IntEnum.ienum5.value == Test.ShortConst1)
    test(Test.IntEnum.ienum6.value == Test.ShortConst1 + 1)
    test(Test.IntEnum.ienum7.value == Test.IntConst1)
    test(Test.IntEnum.ienum8.value == Test.IntConst1 + 1)
    test(Test.IntEnum.ienum9.value == Test.LongConst1)
    test(Test.IntEnum.ienum10.value == Test.LongConst1 + 1)
    test(Test.IntEnum.ienum11.value == Test.IntConst2)
    test(Test.IntEnum.ienum12.value == Test.LongConst2)

    test(Test.IntEnum(0) == Test.IntEnum.ienum1)
    test(Test.IntEnum(1) == Test.IntEnum.ienum2)
    test(Test.IntEnum(Test.ByteConst1) == Test.IntEnum.ienum3)
    test(Test.IntEnum(Test.ByteConst1 + 1) == Test.IntEnum.ienum4)
    test(Test.IntEnum(Test.ShortConst1) == Test.IntEnum.ienum5)
    test(Test.IntEnum(Test.ShortConst1 + 1) == Test.IntEnum.ienum6)
    test(Test.IntEnum(Test.IntConst1) == Test.IntEnum.ienum7)
    test(Test.IntEnum(Test.IntConst1 + 1) == Test.IntEnum.ienum8)
    test(Test.IntEnum(Test.LongConst1) == Test.IntEnum.ienum9)
    test(Test.IntEnum(Test.LongConst1 + 1) == Test.IntEnum.ienum10)
    test(Test.IntEnum(Test.IntConst2) == Test.IntEnum.ienum11)
    test(Test.IntEnum(Test.LongConst2) == Test.IntEnum.ienum12)

    test(Test.SimpleEnum.red.value == 0)
    test(Test.SimpleEnum.green.value == 1)
    test(Test.SimpleEnum.blue.value == 2)

    test(Test.SimpleEnum(0) == Test.SimpleEnum.red)
    test(Test.SimpleEnum(1) == Test.SimpleEnum.green)
    test(Test.SimpleEnum(2) == Test.SimpleEnum.blue)

    print("ok")

    sys.stdout.write("testing enum operations... ")
    sys.stdout.flush()

    test(proxy.opByte(Test.ByteEnum.benum1) == (Test.ByteEnum.benum1, Test.ByteEnum.benum1))
    test(proxy.opByte(Test.ByteEnum.benum11) == (Test.ByteEnum.benum11, Test.ByteEnum.benum11))

    test(proxy.opShort(Test.ShortEnum.senum1) == (Test.ShortEnum.senum1, Test.ShortEnum.senum1))
    test(proxy.opShort(Test.ShortEnum.senum11) == (Test.ShortEnum.senum11, Test.ShortEnum.senum11))

    test(proxy.opInt(Test.IntEnum.ienum1) == (Test.IntEnum.ienum1, Test.IntEnum.ienum1))
    test(proxy.opInt(Test.IntEnum.ienum11) == (Test.IntEnum.ienum11, Test.IntEnum.ienum11))
    test(proxy.opInt(Test.IntEnum.ienum12) == (Test.IntEnum.ienum12, Test.IntEnum.ienum12))

    test(proxy.opSimple(Test.SimpleEnum.green) == (Test.SimpleEnum.green, Test.SimpleEnum.green))

    print("ok")

    sys.stdout.write("testing enum sequences operations... ")
    sys.stdout.flush()

    b1 = [
        Test.ByteEnum.benum1,
        Test.ByteEnum.benum2,
        Test.ByteEnum.benum3,
        Test.ByteEnum.benum4,
        Test.ByteEnum.benum5,
        Test.ByteEnum.benum6,
        Test.ByteEnum.benum7,
        Test.ByteEnum.benum8,
        Test.ByteEnum.benum9,
        Test.ByteEnum.benum10,
        Test.ByteEnum.benum11,
    ]

    (b2, b3) = proxy.opByteSeq(b1)

    for i in range(len(b1)):
        test(b1[i] == b2[i])
        test(b1[i] == b3[i])

    s1 = [
        Test.ShortEnum.senum1,
        Test.ShortEnum.senum2,
        Test.ShortEnum.senum3,
        Test.ShortEnum.senum4,
        Test.ShortEnum.senum5,
        Test.ShortEnum.senum6,
        Test.ShortEnum.senum7,
        Test.ShortEnum.senum8,
        Test.ShortEnum.senum9,
        Test.ShortEnum.senum10,
        Test.ShortEnum.senum11,
    ]

    (s2, s3) = proxy.opShortSeq(s1)

    for i in range(len(s1)):
        test(s1[i] == s2[i])
        test(s1[i] == s3[i])

    i1 = [
        Test.IntEnum.ienum1,
        Test.IntEnum.ienum2,
        Test.IntEnum.ienum3,
        Test.IntEnum.ienum4,
        Test.IntEnum.ienum5,
        Test.IntEnum.ienum6,
        Test.IntEnum.ienum7,
        Test.IntEnum.ienum8,
        Test.IntEnum.ienum9,
        Test.IntEnum.ienum10,
        Test.IntEnum.ienum11,
    ]

    (i2, i3) = proxy.opIntSeq(i1)

    for i in range(len(i1)):
        test(i1[i] == i2[i])
        test(i1[i] == i3[i])

    s1 = [Test.SimpleEnum.red, Test.SimpleEnum.green, Test.SimpleEnum.blue]

    (s2, s3) = proxy.opSimpleSeq(s1)

    for i in range(len(s1)):
        test(s1[i] == s2[i])
        test(s1[i] == s3[i])
    print("ok")

    return proxy
