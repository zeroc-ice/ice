# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, math, Test, array, sys

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def twoways(communicator, p):
    #
    # ice_ping
    #
    p.ice_ping()

    #
    # ice_isA
    #
    test(p.ice_isA(Test.MyClass.ice_staticId()))

    #
    # ice_ids
    #
    ids = p.ice_ids()
    test(len(ids) == 3)
    test(ids[0] == "::Ice::Object")
    test(ids[1] == "::Test::MyClass")
    test(ids[2] == "::Test::MyDerivedClass")

    #
    # ice_id
    #
    test(p.ice_id() == Test.MyDerivedClass.ice_staticId())

    #
    # Prx ice_staticId
    #
    test(Test.MyClassPrx.ice_staticId() == Test.MyClass.ice_staticId())
    test(Test.MyDerivedClassPrx.ice_staticId() == Test.MyDerivedClass.ice_staticId())
    test(Ice.ObjectPrx.ice_staticId() == Ice.Object.ice_staticId())

    #
    # opVoid
    #
    p.opVoid()

    #
    # opByte
    #
    r, b = p.opByte(0xff, 0x0f)
    test(b == 0xf0)
    test(r == 0xff)

    #
    # opBool
    #
    r, b = p.opBool(True, False)
    test(b)
    test(not r)

    #
    # opShortIntLong
    #
    r, s, i, l = p.opShortIntLong(10, 11, 12)
    test(s == 10)
    test(i == 11)
    test(l == 12)
    test(r == 12)

    r, s, i, l = p.opShortIntLong(-32768, -2147483648, -9223372036854775808)
    test(s == -32768)
    test(i == -2147483648)
    test(l == -9223372036854775808)
    test(r == -9223372036854775808)

    r, s, i, l = p.opShortIntLong(32767, 2147483647, 9223372036854775807)
    test(s == 32767)
    test(i == 2147483647)
    test(l == 9223372036854775807)
    test(r == 9223372036854775807)

    #
    # opFloatDouble
    #
    r, f, d = p.opFloatDouble(3.14, 1.1E10)
    test(f - 3.14 < 0.001)
    test(d == 1.1E10)
    test(r == 1.1E10)

    #
    # Test invalid ranges for numbers
    #
    try:
        r, b = p.opByte(0x01ff, 0x01ff)
        test(False)
    except ValueError:
        pass

    try:
        r, s, i, l = p.opShortIntLong(32767 + 1, 0, 0)
        test(False)
    except ValueError:
        pass

    try:
        r, s, i, l = p.opShortIntLong(-32768 -1, 0, 0)
        test(False)
    except ValueError:
        pass

    try:
        r, s, i, l = p.opShortIntLong(0, 2147483647 + 1, 0)
        test(False)
    except ValueError:
        pass

    try:
        r, s, i, l = p.opShortIntLong(0, -2147483648 - 1, 0)
        test(False)
    except ValueError:
        pass

    try:
        r, s, i, l = p.opShortIntLong(0, 0, 9223372036854775807 + 1)
        test(False)
    except ValueError:
        pass

    try:
        r, s, i, l = p.opShortIntLong(0, 0, -9223372036854775808 - 1)
        test(False)
    except ValueError:
        pass

    #
    # opString
    #
    r, s = p.opString("hello", "world")
    test(s == "world hello")
    test(r == "hello world")
    if sys.version_info[0] == 2:
        r, s = p.opString(unicode("hello"), unicode("world"))
        test(s == "world hello")
        test(r == "hello world")

    #
    # opMyEnum
    #
    r, e = p.opMyEnum(Test.MyEnum.enum2)
    test(e == Test.MyEnum.enum2)
    test(r == Test.MyEnum.enum3)

    #
    # opMyClass
    #
    r, c1, c2 = p.opMyClass(p)
    test(Ice.proxyIdentityAndFacetEqual(c1, p))
    test(not Ice.proxyIdentityAndFacetEqual(c2, p))
    test(Ice.proxyIdentityAndFacetEqual(r, p))
    test(c1.ice_getIdentity() == communicator.stringToIdentity("test"))
    test(c2.ice_getIdentity() == communicator.stringToIdentity("noSuchIdentity"))
    test(r.ice_getIdentity() == communicator.stringToIdentity("test"))
    r.opVoid()
    c1.opVoid()
    try:
        c2.opVoid()
        test(False)
    except Ice.ObjectNotExistException:
        pass

    r, c1, c2 = p.opMyClass(None)
    test(not c1)
    test(c2)
    test(Ice.proxyIdentityAndFacetEqual(r, p))
    r.opVoid()

    #
    # opStruct
    #
    si1 = Test.Structure()
    si1.p = p
    si1.e = Test.MyEnum.enum3
    si1.s = Test.AnotherStruct()
    si1.s.s = "abc"
    si2 = Test.Structure()
    si2.p = None
    si2.e = Test.MyEnum.enum2
    si2.s = Test.AnotherStruct()
    si2.s.s = "def"

    rso, so = p.opStruct(si1, si2)
    test(not rso.p)
    test(rso.e == Test.MyEnum.enum2)
    test(rso.s.s == "def")
    test(so.p == p)
    test(so.e == Test.MyEnum.enum3)
    test(so.s.s == "a new string")
    so.p.opVoid()

    #
    # opByteS
    #
    bsi1 = (0x01, 0x11, 0x12, 0x22)
    bsi2 = (0xf1, 0xf2, 0xf3, 0xf4)

    rso, bso = p.opByteS(bsi1, bsi2)
    test(len(bso) == 4)
    test(len(rso) == 8)
    if sys.version_info[0] == 2:
        test(bso[0] == '\x22')
        test(bso[1] == '\x12')
        test(bso[2] == '\x11')
        test(bso[3] == '\x01')
        test(rso[0] == '\x01')
        test(rso[1] == '\x11')
        test(rso[2] == '\x12')
        test(rso[3] == '\x22')
        test(rso[4] == '\xf1')
        test(rso[5] == '\xf2')
        test(rso[6] == '\xf3')
        test(rso[7] == '\xf4')
    else:
        test(bso[0] == 0x22)
        test(bso[1] == 0x12)
        test(bso[2] == 0x11)
        test(bso[3] == 0x01)
        test(rso[0] == 0x01)
        test(rso[1] == 0x11)
        test(rso[2] == 0x12)
        test(rso[3] == 0x22)
        test(rso[4] == 0xf1)
        test(rso[5] == 0xf2)
        test(rso[6] == 0xf3)
        test(rso[7] == 0xf4)

    #
    # opByteS (array)
    #
    bsi1 = array.array('B')
    bsi1.fromlist([0x01, 0x11, 0x12, 0x22])
    bsi2 = array.array('B')
    bsi2.fromlist([0xf1, 0xf2, 0xf3, 0xf4])

    rso, bso = p.opByteS(bsi1, bsi2)
    test(len(bso) == 4)
    test(len(rso) == 8)
    if sys.version_info[0] == 2:
        test(bso[0] == '\x22')
        test(bso[1] == '\x12')
        test(bso[2] == '\x11')
        test(bso[3] == '\x01')
        test(rso[0] == '\x01')
        test(rso[1] == '\x11')
        test(rso[2] == '\x12')
        test(rso[3] == '\x22')
        test(rso[4] == '\xf1')
        test(rso[5] == '\xf2')
        test(rso[6] == '\xf3')
        test(rso[7] == '\xf4')
    else:
        test(bso[0] == 0x22)
        test(bso[1] == 0x12)
        test(bso[2] == 0x11)
        test(bso[3] == 0x01)
        test(rso[0] == 0x01)
        test(rso[1] == 0x11)
        test(rso[2] == 0x12)
        test(rso[3] == 0x22)
        test(rso[4] == 0xf1)
        test(rso[5] == 0xf2)
        test(rso[6] == 0xf3)
        test(rso[7] == 0xf4)

    #
    # opBoolS
    #
    bsi1 = (True, True, False)
    bsi2 = (False,)

    rso, bso = p.opBoolS(bsi1, bsi2)
    test(len(bso) == 4)
    test(bso[0])
    test(bso[1])
    test(not bso[2])
    test(not bso[3])
    test(len(rso) == 3)
    test(not rso[0])
    test(rso[1])
    test(rso[2])

    #
    # opBoolS (array)
    #
    bsi1 = array.array('B')
    bsi1.fromlist([1, 1, 0])
    bsi2 = array.array('B')
    bsi2.fromlist([0])

    rso, bso = p.opBoolS(bsi1, bsi2)
    test(len(bso) == 4)
    test(bso[0])
    test(bso[1])
    test(not bso[2])
    test(not bso[3])
    test(len(rso) == 3)
    test(not rso[0])
    test(rso[1])
    test(rso[2])

    #
    # opShortIntLongS
    #
    ssi = (1, 2, 3)
    isi = (5, 6, 7, 8)
    lsi = (10, 30, 20)

    rso, sso, iso, lso = p.opShortIntLongS(ssi, isi, lsi)
    test(len(sso) == 3)
    test(sso[0] == 1)
    test(sso[1] == 2)
    test(sso[2] == 3)
    test(len(iso) == 4)
    test(iso[0] == 8)
    test(iso[1] == 7)
    test(iso[2] == 6)
    test(iso[3] == 5)
    test(len(lso) == 6)
    test(lso[0] == 10)
    test(lso[1] == 30)
    test(lso[2] == 20)
    test(lso[3] == 10)
    test(lso[4] == 30)
    test(lso[5] == 20)
    test(len(rso) == 3)
    test(rso[0] == 10)
    test(rso[1] == 30)
    test(rso[2] == 20)

    #
    # opShortIntLongS (array)
    #
    ssi = array.array('h')
    ssi.fromlist([1, 2, 3])
    isi = array.array('i')
    isi.fromlist([5, 6, 7, 8])
    lsi = (10, 30, 20)  # Can't store Ice::Long in an array.

    rso, sso, iso, lso = p.opShortIntLongS(ssi, isi, lsi)
    test(len(sso) == 3)
    test(sso[0] == 1)
    test(sso[1] == 2)
    test(sso[2] == 3)
    test(len(iso) == 4)
    test(iso[0] == 8)
    test(iso[1] == 7)
    test(iso[2] == 6)
    test(iso[3] == 5)
    test(len(lso) == 6)
    test(lso[0] == 10)
    test(lso[1] == 30)
    test(lso[2] == 20)
    test(lso[3] == 10)
    test(lso[4] == 30)
    test(lso[5] == 20)
    test(len(rso) == 3)
    test(rso[0] == 10)
    test(rso[1] == 30)
    test(rso[2] == 20)

    #
    # opFloatDoubleS
    #
    fsi = (3.14, 1.11)
    dsi = (1.1E10, 1.2E10, 1.3E10)

    rso, fso, dso = p.opFloatDoubleS(fsi, dsi)
    test(len(fso) == 2)
    test(fso[0] - 3.14 < 0.001)
    test(fso[1] - 1.11 < 0.001)
    test(len(dso) == 3)
    test(dso[0] == 1.3E10)
    test(dso[1] == 1.2E10)
    test(dso[2] == 1.1E10)
    test(len(rso) == 5)
    test(rso[0] == 1.1E10)
    test(rso[1] == 1.2E10)
    test(rso[2] == 1.3E10)
    test(rso[3] - 3.14 < 0.001)
    test(rso[4] - 1.11 < 0.001)

    #
    # opFloatDoubleS (array)
    #
    fsi = array.array('f')
    fsi.fromlist([3.14, 1.11])
    dsi = array.array('d')
    dsi.fromlist([1.1E10, 1.2E10, 1.3E10])

    rso, fso, dso = p.opFloatDoubleS(fsi, dsi)
    test(len(fso) == 2)
    test(fso[0] - 3.14 < 0.001)
    test(fso[1] - 1.11 < 0.001)
    test(len(dso) == 3)
    test(dso[0] == 1.3E10)
    test(dso[1] == 1.2E10)
    test(dso[2] == 1.1E10)
    test(len(rso) == 5)
    test(rso[0] == 1.1E10)
    test(rso[1] == 1.2E10)
    test(rso[2] == 1.3E10)
    test(rso[3] - 3.14 < 0.001)
    test(rso[4] - 1.11 < 0.001)

    #
    # opStringS
    #
    ssi1 = ('abc', 'de', 'fghi')
    ssi2 = ('xyz',)

    rso, sso = p.opStringS(ssi1, ssi2)
    test(len(sso) == 4)
    test(sso[0] == "abc")
    test(sso[1] == "de")
    test(sso[2] == "fghi")
    test(sso[3] == "xyz")
    test(len(rso) == 3)
    test(rso[0] == "fghi")
    test(rso[1] == "de")
    test(rso[2] == "abc")

    #
    # opByteSS
    #
    bsi1 = ((0x01, 0x11, 0x12), (0xff,))
    bsi2 = ((0x0e,), (0xf2, 0xf1))

    rso, bso = p.opByteSS(bsi1, bsi2)
    test(len(bso) == 2)
    test(len(bso[0]) == 1)
    test(len(bso[1]) == 3)
    test(len(rso) == 4)
    test(len(rso[0]) == 3)
    test(len(rso[1]) == 1)
    test(len(rso[2]) == 1)
    test(len(rso[3]) == 2)
    if sys.version_info[0] == 2:
        test(bso[0][0] == '\xff')
        test(bso[1][0] == '\x01')
        test(bso[1][1] == '\x11')
        test(bso[1][2] == '\x12')
        test(rso[0][0] == '\x01')
        test(rso[0][1] == '\x11')
        test(rso[0][2] == '\x12')
        test(rso[1][0] == '\xff')
        test(rso[2][0] == '\x0e')
        test(rso[3][0] == '\xf2')
        test(rso[3][1] == '\xf1')
    else:
        test(bso[0][0] == 0xff)
        test(bso[1][0] == 0x01)
        test(bso[1][1] == 0x11)
        test(bso[1][2] == 0x12)
        test(rso[0][0] == 0x01)
        test(rso[0][1] == 0x11)
        test(rso[0][2] == 0x12)
        test(rso[1][0] == 0xff)
        test(rso[2][0] == 0x0e)
        test(rso[3][0] == 0xf2)
        test(rso[3][1] == 0xf1)

    #
    # opBoolSS
    #
    bsi1 = ((True,), (False,), (True, True),)
    bsi2 = ((False, False, True),)

    rso, bso = p.opBoolSS(bsi1, bsi2)
    test(len(bso) == 4)
    test(len(bso[0]) == 1)
    test(bso[0][0])
    test(len(bso[1]) == 1)
    test(not bso[1][0])
    test(len(bso[2]) == 2)
    test(bso[2][0])
    test(bso[2][1])
    test(len(bso[3]) == 3)
    test(not bso[3][0])
    test(not bso[3][1])
    test(bso[3][2])
    test(len(rso) == 3)
    test(len(rso[0]) == 2)
    test(rso[0][0])
    test(rso[0][1])
    test(len(rso[1]) == 1)
    test(not rso[1][0])
    test(len(rso[2]) == 1)
    test(rso[2][0])

    #
    # opShortIntLongSS
    #
    ssi = ((1,2,5), (13,), ())
    isi = ((24, 98), (42,))
    lsi = ((496, 1729),)

    rso, sso, iso, lso = p.opShortIntLongSS(ssi, isi, lsi)
    test(len(rso) == 1)
    test(len(rso[0]) == 2)
    test(rso[0][0] == 496)
    test(rso[0][1] == 1729)
    test(len(sso) == 3)
    test(len(sso[0]) == 3)
    test(sso[0][0] == 1)
    test(sso[0][1] == 2)
    test(sso[0][2] == 5)
    test(len(sso[1]) == 1)
    test(sso[1][0] == 13)
    test(len(sso[2]) == 0)
    test(len(iso) == 2)
    test(len(iso[0]) == 1)
    test(iso[0][0] == 42)
    test(len(iso[1]) == 2)
    test(iso[1][0] == 24)
    test(iso[1][1] == 98)
    test(len(lso) == 2)
    test(len(lso[0]) == 2)
    test(lso[0][0] == 496)
    test(lso[0][1] == 1729)
    test(len(lso[1]) == 2)
    test(lso[1][0] == 496)
    test(lso[1][1] == 1729)

    #
    # opFloatDoubleSS
    #
    fsi = ((3.14,), (1.11,), ())
    dsi = ((1.1E10, 1.2E10, 1.3E10),)

    rso, fso, dso = p.opFloatDoubleSS(fsi, dsi)
    test(len(fso) == 3)
    test(len(fso[0]) == 1)
    test(fso[0][0] - 3.14 < 0.001)
    test(len(fso[1]) == 1)
    test(fso[1][0] - 1.11 < 0.001)
    test(len(fso[2]) == 0)
    test(len(dso) == 1)
    test(len(dso[0]) == 3)
    test(dso[0][0] == 1.1E10)
    test(dso[0][1] == 1.2E10)
    test(dso[0][2] == 1.3E10)
    test(len(rso) == 2)
    test(len(rso[0]) == 3)
    test(rso[0][0] == 1.1E10)
    test(rso[0][1] == 1.2E10)
    test(rso[0][2] == 1.3E10)
    test(len(rso[1]) == 3)
    test(rso[1][0] == 1.1E10)
    test(rso[1][1] == 1.2E10)
    test(rso[1][2] == 1.3E10)

    #
    # opStringSS
    #
    ssi1 = (('abc',), ('de', 'fghi'))
    ssi2 = ((), (), ('xyz',))

    rso, sso = p.opStringSS(ssi1, ssi2)
    test(len(sso) == 5)
    test(len(sso[0]) == 1)
    test(sso[0][0] == "abc")
    test(len(sso[1]) == 2)
    test(sso[1][0] == "de")
    test(sso[1][1] == "fghi")
    test(len(sso[2]) == 0)
    test(len(sso[3]) == 0)
    test(len(sso[4]) == 1)
    test(sso[4][0] == "xyz")
    test(len(rso) == 3)
    test(len(rso[0]) == 1)
    test(rso[0][0] == "xyz")
    test(len(rso[1]) == 0)
    test(len(rso[2]) == 0)

    #
    # opStringSSS
    #
    sssi1 = ((('abc', 'de'), ('xyz',)), (('hello',),))
    sssi2 = ((('', ''), ('abcd',)), (('',),), ())

    rsso, ssso = p.opStringSSS(sssi1, sssi2)
    test(len(ssso) == 5)
    test(len(ssso[0]) == 2)
    test(len(ssso[0][0]) == 2)
    test(len(ssso[0][1]) == 1)
    test(len(ssso[1]) == 1)
    test(len(ssso[1][0]) == 1)
    test(len(ssso[2]) == 2)
    test(len(ssso[2][0]) == 2)
    test(len(ssso[2][1]) == 1)
    test(len(ssso[3]) == 1)
    test(len(ssso[3][0]) == 1)
    test(len(ssso[4]) == 0)
    test(ssso[0][0][0] == "abc")
    test(ssso[0][0][1] == "de")
    test(ssso[0][1][0] == "xyz")
    test(ssso[1][0][0] == "hello")
    test(ssso[2][0][0] == "")
    test(ssso[2][0][1] == "")
    test(ssso[2][1][0] == "abcd")
    test(ssso[3][0][0] == "")

    test(len(rsso) == 3)
    test(len(rsso[0]) == 0)
    test(len(rsso[1]) == 1)
    test(len(rsso[1][0]) == 1)
    test(len(rsso[2]) == 2)
    test(len(rsso[2][0]) == 2)
    test(len(rsso[2][1]) == 1)
    test(rsso[1][0][0] == "")
    test(rsso[2][0][0] == "")
    test(rsso[2][0][1] == "")
    test(rsso[2][1][0] == "abcd")

    #
    # opByteBoolD
    #
    di1 = {10: True, 100: False}
    di2 = {10: True, 11: False, 101: True}

    ro, do = p.opByteBoolD(di1, di2)

    test(do == di1)
    test(len(ro) == 4)
    test(ro[10])
    test(not ro[11])
    test(not ro[100])
    test(ro[101])

    #
    # opShortIntD
    #
    di1 = {110: -1, 1100: 123123}
    di2 = {110: -1, 111: -100, 1101: 0}

    ro, do = p.opShortIntD(di1, di2)

    test(do == di1)
    test(len(ro) == 4)
    test(ro[110] == -1)
    test(ro[111] == -100)
    test(ro[1100] == 123123)
    test(ro[1101] == 0)

    #
    # opLongFloatD
    #
    di1 = {999999110: -1.1, 999999111: 123123.2}
    di2 = {999999110: -1.1, 999999120: -100.4, 999999130: 0.5}

    ro, do = p.opLongFloatD(di1, di2)

    for k in do:
        test(math.fabs(do[k] - di1[k]) < 0.01)
    test(len(ro) == 4)
    test(ro[999999110] - -1.1 < 0.01)
    test(ro[999999120] - -100.4 < 0.01)
    test(ro[999999111] - 123123.2 < 0.01)
    test(ro[999999130] - 0.5 < 0.01)

    #
    # opStringStringD
    #
    di1 = {'foo': 'abc -1.1', 'bar': 'abc 123123.2'}
    di2 = {'foo': 'abc -1.1', 'FOO': 'abc -100.4', 'BAR': 'abc 0.5'}

    ro, do = p.opStringStringD(di1, di2)

    test(do == di1)
    test(len(ro) == 4)
    test(ro["foo"] == "abc -1.1")
    test(ro["FOO"] == "abc -100.4")
    test(ro["bar"] == "abc 123123.2")
    test(ro["BAR"] == "abc 0.5")

    #
    # opStringMyEnumD
    #
    di1 = {'abc': Test.MyEnum.enum1, '': Test.MyEnum.enum2}
    di2 = {'abc': Test.MyEnum.enum1, 'qwerty': Test.MyEnum.enum3, 'Hello!!': Test.MyEnum.enum2}

    ro, do = p.opStringMyEnumD(di1, di2)

    test(do == di1)
    test(len(ro) == 4)
    test(ro["abc"] == Test.MyEnum.enum1)
    test(ro["qwerty"] == Test.MyEnum.enum3)
    test(ro[""] == Test.MyEnum.enum2)
    test(ro["Hello!!"] == Test.MyEnum.enum2)

    #
    # opMyEnumStringD
    #
    di1 = {Test.MyEnum.enum1: 'abc'}
    di2 = {Test.MyEnum.enum2: 'Hello!!', Test.MyEnum.enum3: 'qwerty'}

    ro, do = p.opMyEnumStringD(di1, di2)

    test(do == di1)
    test(len(ro) == 3)
    test(ro[Test.MyEnum.enum1] == "abc")
    test(ro[Test.MyEnum.enum2] == "Hello!!")
    test(ro[Test.MyEnum.enum3] == "qwerty")

    #
    # opMyStructMyEnumD
    #
    s11 = Test.MyStruct()
    s11.i = 1
    s11.j = 1
    s12 = Test.MyStruct()
    s12.i = 1
    s12.j = 2
    s22 = Test.MyStruct()
    s22.i = 2
    s22.j = 2
    s23 = Test.MyStruct()
    s23.i = 2
    s23.j = 3
    di1 = {s11: Test.MyEnum.enum1, s12: Test.MyEnum.enum2}
    di2 = {s11: Test.MyEnum.enum1, s22: Test.MyEnum.enum3, s23: Test.MyEnum.enum2}

    ro, do = p.opMyStructMyEnumD(di1, di2)

    test(do == di1)
    test(len(ro) == 4)
    test(ro[s11] == Test.MyEnum.enum1)
    test(ro[s12] == Test.MyEnum.enum2)
    test(ro[s22] == Test.MyEnum.enum3)
    test(ro[s23] == Test.MyEnum.enum2)

    #
    # opByteBoolDS
    #
    dsi1 = ({ 10: True, 100: False }, { 10: True, 11: False, 101: True })
    dsi2 = ({ 100: False, 101: False },)

    ro, do = p.opByteBoolDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 3)
    test(ro[0][10])
    test(not ro[0][11])
    test(ro[0][101])
    test(len(ro[1]) == 2)
    test(ro[1][10])
    test(not ro[1][100])
    test(len(do) == 3)
    test(len(do[0]) == 2)
    test(not do[0][100])
    test(not do[0][101])
    test(len(do[1]) == 2)
    test(do[1][10])
    test(not do[1][100])
    test(len(do[2]) == 3)
    test(do[2][10])
    test(not do[2][11])
    test(do[2][101])

    #
    # opShortIntDS
    #
    dsi1 = ({ 110: -1, 1100: 123123 }, { 110: -1, 111: -100, 1101: 0 })
    dsi2 = ({ 100: -1001 },)

    ro, do = p.opShortIntDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 3)
    test(ro[0][110] == -1)
    test(ro[0][111] == -100)
    test(ro[0][1101] == 0)
    test(len(ro[1]) == 2)
    test(ro[1][110] == -1)
    test(ro[1][1100] == 123123)

    test(len(do) == 3)
    test(len(do[0]) == 1)
    test(do[0][100] == -1001)
    test(len(do[1]) == 2)
    test(do[1][110] == -1)
    test(do[1][1100] == 123123)
    test(len(do[2]) == 3)
    test(do[2][110] == -1)
    test(do[2][111] == -100)
    test(do[2][1101] == 0)

    #
    # opLongFloatDS
    #
    dsi1 = ({ 999999110: -1.1, 999999111: 123123.2 }, { 999999110: -1.1, 999999120: -100.4, 999999130: 0.5 })
    dsi2 = ({ 999999140: 3.14 },)

    ro, do = p.opLongFloatDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 3)
    test(ro[0][999999110] - -1.1 < 0.01)
    test(ro[0][999999120] - -100.4 < 0.01)
    test(ro[0][999999130] - 0.5 < 0.01)
    test(len(ro[1]) == 2)
    test(ro[1][999999110] - -1.1 < 0.01)
    test(ro[1][999999111] - 123123.2 < 0.01)

    test(len(do) == 3)
    test(len(do[0]) == 1)
    test(do[0][999999140] - 3.14 < 0.01)
    test(len(do[1]) == 2)
    test(do[1][999999110] - -1.1 < 0.01)
    test(do[1][999999111] - 123123.2 < 0.01)
    test(len(do[2]) == 3)
    test(do[2][999999110] - -1.1 < 0.01)
    test(do[2][999999120] - -100.4 < 0.01)
    test(do[2][999999130] - 0.5 < 0.01)

    #
    # opStringStringDS
    #

    dsi1 = ({ "foo": "abc -1.1", "bar": "abc 123123.2" }, { "foo": "abc -1.1", "FOO": "abc -100.4", "BAR": "abc 0.5" })
    dsi2 = ({ "f00": "ABC -3.14" },)

    ro, do = p.opStringStringDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 3)
    test(ro[0]["foo"] == "abc -1.1")
    test(ro[0]["FOO"] == "abc -100.4")
    test(ro[0]["BAR"] == "abc 0.5")
    test(len(ro[1]) == 2)
    test(ro[1]["foo"] == "abc -1.1")
    test(ro[1]["bar"] == "abc 123123.2")

    test(len(do) == 3)
    test(len(do[0]) == 1)
    test(do[0]["f00"] == "ABC -3.14")
    test(len(do[1]) == 2)
    test(do[1]["foo"] == "abc -1.1")
    test(do[1]["bar"] == "abc 123123.2")
    test(len(do[2]) == 3)
    test(do[2]["foo"] == "abc -1.1")
    test(do[2]["FOO"] == "abc -100.4")
    test(do[2]["BAR"] == "abc 0.5")

    #
    # opStringMyEnumDS
    #
    dsi1 = (
            { "abc": Test.MyEnum.enum1, "": Test.MyEnum.enum2 },
            { "abc": Test.MyEnum.enum1, "qwerty": Test.MyEnum.enum3, "Hello!!": Test.MyEnum.enum2 }
           )

    dsi2 = ({ "Goodbye": Test.MyEnum.enum1 },)

    ro, do = p.opStringMyEnumDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 3)
    test(ro[0]["abc"] == Test.MyEnum.enum1)
    test(ro[0]["qwerty"] == Test.MyEnum.enum3)
    test(ro[0]["Hello!!"] == Test.MyEnum.enum2)
    test(len(ro[1]) == 2)
    test(ro[1]["abc"] == Test.MyEnum.enum1)
    test(ro[1][""] == Test.MyEnum.enum2)

    test(len(do) == 3)
    test(len(do[0]) == 1)
    test(do[0]["Goodbye"] == Test.MyEnum.enum1)
    test(len(do[1]) == 2)
    test(do[1]["abc"] == Test.MyEnum.enum1)
    test(do[1][""] == Test.MyEnum.enum2)
    test(len(do[2]) == 3)
    test(do[2]["abc"] == Test.MyEnum.enum1)
    test(do[2]["qwerty"] == Test.MyEnum.enum3)
    test(do[2]["Hello!!"] == Test.MyEnum.enum2)

    #
    # opMyEnumStringDS
    #
    dsi1 = ({ Test.MyEnum.enum1: 'abc' }, { Test.MyEnum.enum2: 'Hello!!', Test.MyEnum.enum3: 'qwerty'})
    dsi2 = ({ Test.MyEnum.enum1: 'Goodbye' },)

    ro, do = p.opMyEnumStringDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 2)
    test(ro[0][Test.MyEnum.enum2] == "Hello!!")
    test(ro[0][Test.MyEnum.enum3] == "qwerty")
    test(len(ro[1]) == 1)
    test(ro[1][Test.MyEnum.enum1] == "abc")

    test(len(do) == 3)
    test(len(do[0]) == 1)
    test(do[0][Test.MyEnum.enum1] == "Goodbye")
    test(len(do[1]) == 1)
    test(do[1][Test.MyEnum.enum1] == "abc")
    test(len(do[2]) == 2)
    test(do[2][Test.MyEnum.enum2] == "Hello!!")
    test(do[2][Test.MyEnum.enum3] == "qwerty")

    #
    # opMyStructMyEnumDS
    #
    s11 = Test.MyStruct(1, 1)
    s12 = Test.MyStruct(1, 2)

    s22 = Test.MyStruct(2, 2)
    s23 = Test.MyStruct(2, 3)

    dsi1 = (
            { s11: Test.MyEnum.enum1, s12: Test.MyEnum.enum2 },
            { s11: Test.MyEnum.enum1, s22: Test.MyEnum.enum3, s23: Test.MyEnum.enum2 }
           )
    dsi2 = ({ s23: Test.MyEnum.enum3 },)

    ro, do = p.opMyStructMyEnumDS(dsi1, dsi2)

    test(len(ro) == 2)
    test(len(ro[0]) == 3)
    test(ro[0][s11] == Test.MyEnum.enum1)
    test(ro[0][s22] == Test.MyEnum.enum3)
    test(ro[0][s23] == Test.MyEnum.enum2)
    test(len(ro[1]) == 2)
    test(ro[1][s11] == Test.MyEnum.enum1)
    test(ro[1][s12] == Test.MyEnum.enum2)

    test(len(do) == 3)
    test(len(do[0]) == 1)
    test(do[0][s23] == Test.MyEnum.enum3)
    test(len(do[1]) == 2)
    test(do[1][s11] == Test.MyEnum.enum1)
    test(do[1][s12] == Test.MyEnum.enum2)
    test(len(do[2]) == 3)
    test(do[2][s11] == Test.MyEnum.enum1)
    test(do[2][s22] == Test.MyEnum.enum3)
    test(do[2][s23] == Test.MyEnum.enum2)

    #
    #opByteByteSD
    #
    sdi1 = { 0x01: (0x01, 0x11), 0x22: (0x12,) }
    sdi2 = { 0xf1: (0xf2, 0xf3) }

    ro, do = p.opByteByteSD(sdi1, sdi2)

    if sys.version_info[0] == 2:
        test(len(do) == 1)
        test(len(do[0xf1]) == 2)
        test(do[0xf1][0] == '\xf2')
        test(do[0xf1][1] == '\xf3')
        test(len(ro) == 3)
        test(len(ro[0x01]) == 2)
        test(ro[0x01][0] == '\x01')
        test(ro[0x01][1] == '\x11')
        test(len(ro[0x22]) == 1)
        test(ro[0x22][0] == '\x12')
        test(len(ro[0xf1]) == 2)
        test(ro[0xf1][0] == '\xf2')
        test(ro[0xf1][1] == '\xf3')
    else:
        test(len(do) == 1)
        test(len(do[0xf1]) == 2)
        test(do[0xf1][0] == 0xf2)
        test(do[0xf1][1] == 0xf3)
        test(len(ro) == 3)
        test(len(ro[0x01]) == 2)
        test(ro[0x01][0] == 0x01)
        test(ro[0x01][1] == 0x11)
        test(len(ro[0x22]) == 1)
        test(ro[0x22][0] == 0x12)
        test(len(ro[0xf1]) == 2)
        test(ro[0xf1][0] == 0xf2)
        test(ro[0xf1][1] == 0xf3)

    #
    # opBoolBoolSD
    #
    sdi1 = { False: (True, False), True: (False, True, True) }
    sdi2 = { False: (True, False) }

    ro, do = p.opBoolBoolSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do[False]) == 2)
    test(do[False][0])
    test(not do[False][1])
    test(len(ro) == 2)
    test(len(ro[False]) == 2)
    test(ro[False][0])
    test(not ro[False][1])
    test(len(ro[True]) == 3)
    test(not ro[True][0])
    test(ro[True][1])
    test(ro[True][2])

    #
    # opShortShortSD
    #
    sdi1 = { 1: (1, 2, 3), 2: (4, 5) }
    sdi2 = { 4: (6, 7) }

    ro, do = p.opShortShortSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do[4]) == 2)
    test(do[4][0] == 6)
    test(do[4][1] == 7)
    test(len(ro) == 3)
    test(len(ro[1]) == 3)
    test(ro[1][0] == 1)
    test(ro[1][1] == 2)
    test(ro[1][2] == 3)
    test(len(ro[2]) == 2)
    test(ro[2][0] == 4)
    test(ro[2][1] == 5)
    test(len(ro[4]) == 2)
    test(ro[4][0] == 6)
    test(ro[4][1] == 7)

    #
    # opIntIntSD
    #
    sdi1 = { 100: (100, 200, 300), 200: (400, 500) }
    sdi2 = { 400: (600, 700) }

    ro, do = p.opIntIntSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do[400]) == 2)
    test(do[400][0] == 600)
    test(do[400][1] == 700)
    test(len(ro) == 3)
    test(len(ro[100]) == 3)
    test(ro[100][0] == 100)
    test(ro[100][1] == 200)
    test(ro[100][2] == 300)
    test(len(ro[200]) == 2)
    test(ro[200][0] == 400)
    test(ro[200][1] == 500)
    test(len(ro[400]) == 2)
    test(ro[400][0] == 600)
    test(ro[400][1] == 700)

    #
    # opLongLongSD
    #
    sdi1 = { 999999990: (999999110, 999999111, 999999110), 999999991: (999999120, 999999130) }
    sdi2 = { 999999992: (999999110, 999999120) }

    ro, do = p.opLongLongSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do[999999992]) == 2)
    test(do[999999992][0] == 999999110)
    test(do[999999992][1] == 999999120)
    test(len(ro) == 3)
    test(len(ro[999999990]) == 3)
    test(ro[999999990][0] == 999999110)
    test(ro[999999990][1] == 999999111)
    test(ro[999999990][2] == 999999110)
    test(len(ro[999999991]) == 2)
    test(ro[999999991][0] == 999999120)
    test(ro[999999991][1] == 999999130)
    test(len(ro[999999992]) == 2)
    test(ro[999999992][0] == 999999110)
    test(ro[999999992][1] == 999999120)

    #
    # opStringFloatSD
    #
    sdi1 = { "abc": (-1.1, 123123.2, 100.0), "ABC": (42.24, -1.61) }
    sdi2 = { "aBc": (-3.14, 3.14) }

    ro, do = p.opStringFloatSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do["aBc"]) == 2)
    test(do["aBc"][0] - -3.14 < 0.01)
    test(do["aBc"][1] - 3.14 < 0.01)

    test(len(ro) == 3)
    test(len(ro["abc"]) == 3)
    test(ro["abc"][0] - -1.1 < 0.01)
    test(ro["abc"][1] - 123123.2 < 0.01)
    test(ro["abc"][2] - 100.0 < 0.01)
    test(len(ro["ABC"]) == 2)
    test(ro["ABC"][0] - 42.24 < 0.01)
    test(ro["ABC"][1] - -1.61 < 0.01)
    test(len(ro["aBc"]) == 2)
    test(ro["aBc"][0] - -3.14 < 0.01)
    test(ro["aBc"][1] - 3.14 < 0.01)

    #
    # opStringDoubleSD
    #
    sdi1 = { "Hello!!": (1.1E10, 1.2E10, 1.3E10), "Goodbye": (1.4E10, 1.5E10) }
    sdi2 = { "": (1.6E10, 1.7E10) }

    ro, do = p.opStringDoubleSD(sdi1, sdi2);

    test(len(do) == 1)
    test(len(do[""]) == 2)
    test(do[""][0] == 1.6E10)
    test(do[""][1] == 1.7E10)
    test(len(ro) == 3)
    test(len(ro["Hello!!"]) == 3)
    test(ro["Hello!!"][0] == 1.1E10)
    test(ro["Hello!!"][1] == 1.2E10)
    test(ro["Hello!!"][2] == 1.3E10)
    test(len(ro["Goodbye"]) == 2)
    test(ro["Goodbye"][0] == 1.4E10)
    test(ro["Goodbye"][1] == 1.5E10)
    test(len(ro[""]) == 2)
    test(ro[""][0] == 1.6E10)
    test(ro[""][1] == 1.7E10)

    #
    # opStringStringSD
    #
    sdi1 = { "abc": ("abc", "de", "fghi") , "def": ("xyz", "or") }
    sdi2 = { "ghi": ("and", "xor") }

    ro, do = p.opStringStringSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do["ghi"]) == 2)
    test(do["ghi"][0] == "and")
    test(do["ghi"][1] == "xor")
    test(len(ro) == 3)
    test(len(ro["abc"]) == 3)
    test(ro["abc"][0] == "abc")
    test(ro["abc"][1] == "de")
    test(ro["abc"][2] == "fghi")
    test(len(ro["def"]) == 2)
    test(ro["def"][0] == "xyz")
    test(ro["def"][1] == "or")
    test(len(ro["ghi"]) == 2)
    test(ro["ghi"][0] == "and")
    test(ro["ghi"][1] == "xor")

    #
    # opMyEnumMyEnumSD
    #
    sdi1 = {
            Test.MyEnum.enum3: (Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2),
            Test.MyEnum.enum2: (Test.MyEnum.enum1, Test.MyEnum.enum2)
           }
    sdi2 = { Test.MyEnum.enum1: (Test.MyEnum.enum3, Test.MyEnum.enum3) }

    ro, do = p.opMyEnumMyEnumSD(sdi1, sdi2)

    test(len(do) == 1)
    test(len(do[Test.MyEnum.enum1]) == 2)
    test(do[Test.MyEnum.enum1][0] == Test.MyEnum.enum3)
    test(do[Test.MyEnum.enum1][1] == Test.MyEnum.enum3)
    test(len(ro) == 3)
    test(len(ro[Test.MyEnum.enum3]) == 3)
    test(ro[Test.MyEnum.enum3][0] == Test.MyEnum.enum1)
    test(ro[Test.MyEnum.enum3][1] == Test.MyEnum.enum1)
    test(ro[Test.MyEnum.enum3][2] == Test.MyEnum.enum2)
    test(len(ro[Test.MyEnum.enum2]) == 2)
    test(ro[Test.MyEnum.enum2][0] == Test.MyEnum.enum1)
    test(ro[Test.MyEnum.enum2][1] == Test.MyEnum.enum2)
    test(len(ro[Test.MyEnum.enum1]) == 2)
    test(ro[Test.MyEnum.enum1][0] == Test.MyEnum.enum3)
    test(ro[Test.MyEnum.enum1][1] == Test.MyEnum.enum3)

    #
    # opIntS
    #
    lengths = ( 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 )
    for l in lengths:
        s = []
        for i in range(l):
            s.append(i)
        r = p.opIntS(s)
        test(len(r) == l)
        for j in range(len(r)):
            test(r[j] == -j)


    #
    # opContext
    #
    ctx = {'one': 'ONE', 'two': 'TWO', 'three': 'THREE'}

    r = p.opContext()
    test(len(p.ice_getContext()) == 0)
    test(r != ctx)

    r = p.opContext(ctx)
    test(len(p.ice_getContext()) == 0)
    test(r == ctx)

    p2 = Test.MyClassPrx.checkedCast(p.ice_context(ctx))
    test(p2.ice_getContext() == ctx)
    r = p2.opContext()
    test(r == ctx)
    r = p2.opContext(ctx)
    test(r == ctx)

    #
    # Test implicit context propagation
    #
    if p.ice_getConnection():
        impls = ( 'Shared', 'PerThread' )
        for i in impls:
            initData = Ice.InitializationData()
            initData.properties = communicator.getProperties().clone()
            initData.properties.setProperty('Ice.ImplicitContext', i)
            ic = Ice.initialize(data=initData)

            ctx = {'one': 'ONE', 'two': 'TWO', 'three': 'THREE'}

            p1 = Test.MyClassPrx.uncheckedCast(ic.stringToProxy('test:default -p 12010'))

            ic.getImplicitContext().setContext(ctx)
            test(ic.getImplicitContext().getContext() == ctx)
            test(p1.opContext() == ctx)

            test(ic.getImplicitContext().containsKey('zero') == False)
            r = ic.getImplicitContext().put('zero', 'ZERO')
            test(r == '')
            test(ic.getImplicitContext().containsKey('zero') == True)
            test(ic.getImplicitContext().get('zero') == 'ZERO')

            ctx = ic.getImplicitContext().getContext()
            test(p1.opContext() == ctx)

            prxContext = {'one': 'UN', 'four': 'QUATRE'}

            combined = ctx.copy()
            combined.update(prxContext)
            test(combined['one'] == 'UN')

            p2 = Test.MyClassPrx.uncheckedCast(p1.ice_context(prxContext))

            ic.getImplicitContext().setContext({})
            test(p2.opContext() == prxContext)

            ic.getImplicitContext().setContext(ctx)
            test(p2.opContext() == combined)

            test(ic.getImplicitContext().remove('one') == 'ONE')

            ic.destroy()

    d = 1278312346.0 / 13.0
    ds = []
    for i in range(5):
        ds.append(d)
    p.opDoubleMarshaling(d, ds)

    #
    # opIdempotent
    #
    p.opIdempotent()

    #
    # opNonmutating
    #
    p.opNonmutating()

