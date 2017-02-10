# encoding: utf-8
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


def twoways(communicator, p)

    literals = p.opStringLiterals();

    test(Test::S0 == "\\" &&
         Test::S0 == Test::Sw0 &&
         Test::S0 == literals[0] &&
         Test::S0 == literals[11]);

    test(Test::S1 == "A" &&
         Test::S1 == Test::Sw1 &&
         Test::S1 == literals[1] &&
         Test::S1 == literals[12]);

    test(Test::S2 == "Ice" &&
         Test::S2 == Test::Sw2 &&
         Test::S2 == literals[2] &&
         Test::S2 == literals[13]);

    test(Test::S3 == "A21" &&
         Test::S3 == Test::Sw3 &&
         Test::S3 == literals[3] &&
         Test::S3 == literals[14]);

    test(Test::S4 == "\\u0041 \\U00000041" &&
         Test::S4 == Test::Sw4 &&
         Test::S4 == literals[4] &&
         Test::S4 == literals[15]);

    test(Test::S5 == "\u00FF" &&
         Test::S5 == Test::Sw5 &&
         Test::S5 == literals[5] &&
         Test::S5 == literals[16]);

    test(Test::S6 == "\u03FF" &&
         Test::S6 == Test::Sw6 &&
         Test::S6 == literals[6] &&
         Test::S6 == literals[17]);

    test(Test::S7 == "\u05F0" &&
         Test::S7 == Test::Sw7 &&
         Test::S7 == literals[7] &&
         Test::S7 == literals[18]);

    test(Test::S8 == "\u{10000}" &&
         Test::S8 == Test::Sw8 &&
         Test::S8 == literals[8] &&
         Test::S8 == literals[19]);

    test(Test::S9 == "\u{01F34C}" &&
         Test::S9 == Test::Sw9 &&
         Test::S9 == literals[9] &&
         Test::S9 == literals[20]);

    test(Test::S10 == "\u0DA7" &&
         Test::S10 == Test::Sw10 &&
         Test::S10 == literals[10] &&
         Test::S10 == literals[21]);

    test(Test::Ss0 == "\'\"\x3f\\\a\b\f\n\r\t\v\6" &&
         Test::Ss0 == Test::Ss1 &&
         Test::Ss0 == Test::Ss2 &&
         Test::Ss0 == literals[22] &&
         Test::Ss0 == literals[23] &&
         Test::Ss0 == literals[24]);

    test(Test::Ss3 == "\\\\U\\u\\" &&
         Test::Ss3 == literals[25]);

    test(Test::Ss4 == "\\A\\" &&
         Test::Ss4 == literals[26]);

    test(Test::Ss5 == "\\u0041\\" &&
         Test::Ss5 == literals[27]);

    test(Test::Su0 == Test::Su1 &&
         Test::Su0 == Test::Su2 &&
         Test::Su0 == literals[28] &&
         Test::Su0 == literals[29] &&
         Test::Su0 == literals[30]);

    #
    # ice_ping
    #
    p.ice_ping

    #
    # ice_isA
    #
    test(p.ice_isA("::Test::MyClass"))

    #
    # ice_ids
    #
    ids = p.ice_ids
    test(ids.length == 3)
    test(ids[0] == "::Ice::Object")
    test(ids[1] == "::Test::MyClass")
    test(ids[2] == "::Test::MyDerivedClass")

    #
    # ice_id
    #
    test(p.ice_id == "::Test::MyDerivedClass")

    #
    # Proxy ice_staticId
    #
    test(Test::MyClassPrx::ice_staticId() == "::Test::MyClass")
    test(Test::MyDerivedClassPrx::ice_staticId() == "::Test::MyDerivedClass")
    test(Ice::ObjectPrx::ice_staticId() == Ice::Value::ice_staticId())

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
    r, b = p.opBool(true, false)
    test(b)
    test(!r)

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
    begin
        r, b = p.opByte(0x01ff, 0x01ff)
        test(false)
    rescue TypeError
    end

    begin
        r, s, i, l = p.opShortIntLong(32767 + 1, 0, 0)
        test(false)
    rescue TypeError
    end

    begin
        r, s, i, l = p.opShortIntLong(-32768 - 1, 0, 0)
        test(false)
    rescue TypeError
    end

    begin
        r, s, i, l = p.opShortIntLong(0, 2147483647 + 1, 0)
        test(false)
    rescue TypeError
    end

    begin
        r, s, i, l = p.opShortIntLong(0, -2147483648 - 1, 0)
        test(false)
    rescue TypeError
    end

    begin
        r, s, i, l = p.opShortIntLong(0, 0, 9223372036854775807 + 1)
        test(false)
    rescue TypeError
    end

    begin
        r, s, i, l = p.opShortIntLong(0, 0, -9223372036854775808 - 1)
        test(false)
    rescue TypeError
    end

    r, f, d = p.opFloatDouble(3.402823466E38, 0.0)
    r, f, d = p.opFloatDouble(-3.402823466E38, 0.0)

    #
    # For portability, don't use Float::NAN here.
    #
    nan = 0.0 / 0.0
    for val in [nan, -nan]
        r, f, d = p.opFloatDouble(val, val)
        test(r.nan? && f.nan? && d.nan?)
    end
    #
    # For portability, don't use Float::INFINITY here.
    #
    inf = 1.0 / 0.0
    for val in [inf, -inf]
        r, f, d = p.opFloatDouble(val, val)
        test(r.infinite? != 0 && f.infinite? != 0 && d.infinite? != 0)
    end

    begin
        r, f, d = p.opFloatDouble(3.402823466E38*2, 0.0)
        test(false)
    rescue TypeError
    end

    begin
        r, f, d = p.opFloatDouble(-3.402823466E38*2, 0.0)
        test(false)
    rescue TypeError
    end

    #
    # opString
    #
    r, s = p.opString("hello", "world")
    test(s == "world hello")
    test(r == "hello world")

    #
    # opMyEnum
    #
    r, e = p.opMyEnum(Test::MyEnum::Enum2)
    test(e == Test::MyEnum::Enum2)
    test(r == Test::MyEnum::Enum3)

    #
    # opMyClass
    #
    r, c1, c2 = p.opMyClass(p)
    test(Ice::proxyIdentityAndFacetEqual(c1, p))
    test(!Ice::proxyIdentityAndFacetEqual(c2, p))
    test(Ice::proxyIdentityAndFacetEqual(r, p))
    test(c1.ice_getIdentity() == Ice::stringToIdentity("test"))
    test(c2.ice_getIdentity() == Ice::stringToIdentity("noSuchIdentity"))
    test(r.ice_getIdentity() == Ice::stringToIdentity("test"))
    r.opVoid()
    c1.opVoid()
    begin
        c2.opVoid()
        test(false)
    rescue Ice::ObjectNotExistException
    end

    r, c1, c2 = p.opMyClass(nil)
    test(!c1)
    test(c2)
    test(Ice::proxyIdentityAndFacetEqual(r, p))
    r.opVoid()

    #
    # opStruct
    #
    si1 = Test::Structure.new
    si1.p = p
    si1.e = Test::MyEnum::Enum3
    si1.s = Test::AnotherStruct.new
    si1.s.s = "abc"
    si2 = Test::Structure.new
    si2.p = nil
    si2.e = Test::MyEnum::Enum2
    si2.s = Test::AnotherStruct.new
    si2.s.s = "def"

    rso, so = p.opStruct(si1, si2)
    test(!rso.p)
    test(rso.e == Test::MyEnum::Enum2)
    test(rso.s.s == "def")
    test(so.p == p)
    test(so.e == Test::MyEnum::Enum3)
    test(so.s.s == "a new string")
    so.p.opVoid()

    # Test marshalling of null structs and structs with null members.
    si1 = Test::Structure.new
    si2 = nil

    rso, so = p.opStruct(si1, si2)
    test(!rso.p)
    test(rso.e == Test::MyEnum::Enum1)
    test(rso.s.s == "")
    test(!so.p)
    test(so.e == Test::MyEnum::Enum1)
    test(so.s.s == "a new string")

    #
    # opByteS
    #
    bsi1 = [0x01, 0x11, 0x12, 0x22]
    bsi2 = [0xf1, 0xf2, 0xf3, 0xf4]

    rso, bso = p.opByteS(bsi1, bsi2)
    test(bso.length == 4)
    test(bso.unpack("C*").reverse() == bsi1)
    arr = bsi1.dup().concat(bsi2)
    test(rso.length == 8)
    test(rso.unpack("C*") == arr)

    #
    # opBoolS
    #
    bsi1 = [true, true, false]
    bsi2 = [false]

    rso, bso = p.opBoolS(bsi1, bsi2)
    test(bso.length == 4)
    test(bso[0])
    test(bso[1])
    test(!bso[2])
    test(!bso[3])
    test(rso.length == 3)
    test(!rso[0])
    test(rso[1])
    test(rso[2])

    #
    # opShortIntLongS
    #
    ssi = [1, 2, 3]
    isi = [5, 6, 7, 8]
    lsi = [10, 30, 20]

    rso, sso, iso, lso = p.opShortIntLongS(ssi, isi, lsi)
    test(sso.length == 3)
    test(sso[0] == 1)
    test(sso[1] == 2)
    test(sso[2] == 3)
    test(iso.length == 4)
    test(iso[0] == 8)
    test(iso[1] == 7)
    test(iso[2] == 6)
    test(iso[3] == 5)
    test(lso.length == 6)
    test(lso[0] == 10)
    test(lso[1] == 30)
    test(lso[2] == 20)
    test(lso[3] == 10)
    test(lso[4] == 30)
    test(lso[5] == 20)
    test(rso.length == 3)
    test(rso[0] == 10)
    test(rso[1] == 30)
    test(rso[2] == 20)

    #
    # opFloatDoubleS
    #
    fsi = [3.14, 1.11]
    dsi = [1.1E10, 1.2E10, 1.3E10]

    rso, fso, dso = p.opFloatDoubleS(fsi, dsi)
    test(fso.length == 2)
    test(fso[0] - 3.14 < 0.001)
    test(fso[1] - 1.11 < 0.001)
    test(dso.length == 3)
    test(dso[0] == 1.3E10)
    test(dso[1] == 1.2E10)
    test(dso[2] == 1.1E10)
    test(rso.length == 5)
    test(rso[0] == 1.1E10)
    test(rso[1] == 1.2E10)
    test(rso[2] == 1.3E10)
    test(rso[3] - 3.14 < 0.001)
    test(rso[4] - 1.11 < 0.001)

    #
    # opStringS
    #
    ssi1 = ['abc', 'de', 'fghi']
    ssi2 = ['xyz']

    rso, sso = p.opStringS(ssi1, ssi2)
    test(sso.length == 4)
    test(sso[0] == "abc")
    test(sso[1] == "de")
    test(sso[2] == "fghi")
    test(sso[3] == "xyz")
    test(rso.length == 3)
    test(rso[0] == "fghi")
    test(rso[1] == "de")
    test(rso[2] == "abc")

    #
    # opByteSS
    #
    bsi1 = [[0x01, 0x11, 0x12], [0xff]]
    bsi2 = [[0x0e], [0xf2, 0xf1]]

    rso, bso = p.opByteSS(bsi1, bsi2)
    test(bso.length == 2)
    test(bso[0].length == 1)
    test(bso[0].unpack("C*") == [0xff])
    test(bso[1].length == 3)
    test(bso[1].unpack("C*") == [0x01, 0x11, 0x12])
    test(rso.length == 4)
    test(rso[0].length == 3)
    test(rso[0].unpack("C*") == [0x01, 0x11, 0x12])
    test(rso[1].length == 1)
    test(rso[1].unpack("C*") == [0xff])
    test(rso[2].length == 1)
    test(rso[2].unpack("C*") == [0x0e])
    test(rso[3].length == 2)
    test(rso[3].unpack("C*") == [0xf2, 0xf1])

    #
    # opBoolSS
    #
    bsi1 = [[true], [false], [true, true]];
    bsi2 = [[false, false, true]];

    rso, bso = p.opBoolSS(bsi1, bsi2);
    test(bso.length == 4);
    test(bso[0].length == 1);
    test(bso[0][0]);
    test(bso[1].length == 1);
    test(!bso[1][0]);
    test(bso[2].length == 2);
    test(bso[2][0]);
    test(bso[2][1]);
    test(bso[3].length == 3);
    test(!bso[3][0]);
    test(!bso[3][1]);
    test(bso[3][2]);
    test(rso.length == 3);
    test(rso[0].length == 2);
    test(rso[0][0]);
    test(rso[0][1]);
    test(rso[1].length == 1);
    test(!rso[1][0]);
    test(rso[2].length == 1);
    test(rso[2][0]);

    #
    # opShortIntLongSS
    #
    ssi = [[1, 2, 5], [13], []];
    isi = [[24, 98], [42]];
    lsi = [[496, 1729]];

    rso, sso, iso, lso = p.opShortIntLongSS(ssi, isi, lsi);
    test(rso.length == 1);
    test(rso[0].length == 2);
    test(rso[0][0] == 496);
    test(rso[0][1] == 1729);
    test(sso.length == 3);
    test(sso[0].length == 3);
    test(sso[0][0] == 1);
    test(sso[0][1] == 2);
    test(sso[0][2] == 5);
    test(sso[1].length == 1);
    test(sso[1][0] == 13);
    test(sso[2].length == 0);
    test(iso.length == 2);
    test(iso[0].length == 1);
    test(iso[0][0] == 42);
    test(iso[1].length == 2);
    test(iso[1][0] == 24);
    test(iso[1][1] == 98);
    test(lso.length == 2);
    test(lso[0].length == 2);
    test(lso[0][0] == 496);
    test(lso[0][1] == 1729);
    test(lso[1].length == 2);
    test(lso[1][0] == 496);
    test(lso[1][1] == 1729);

    #
    # opFloatDoubleSS
    #
    fsi = [[3.14], [1.11], []]
    dsi = [[1.1E10, 1.2E10, 1.3E10]]

    rso, fso, dso = p.opFloatDoubleSS(fsi, dsi)
    test(fso.length == 3)
    test(fso[0].length == 1)
    test(fso[0][0] - 3.14 < 0.001)
    test(fso[1].length == 1)
    test(fso[1][0] - 1.11 < 0.001)
    test(fso[2].length == 0)
    test(dso.length == 1)
    test(dso[0].length == 3)
    test(dso[0][0] == 1.1E10)
    test(dso[0][1] == 1.2E10)
    test(dso[0][2] == 1.3E10)
    test(rso.length == 2)
    test(rso[0].length == 3)
    test(rso[0][0] == 1.1E10)
    test(rso[0][1] == 1.2E10)
    test(rso[0][2] == 1.3E10)
    test(rso[1].length == 3)
    test(rso[1][0] == 1.1E10)
    test(rso[1][1] == 1.2E10)
    test(rso[1][2] == 1.3E10)

    #
    # opStringSS
    #
    ssi1 = [['abc'], ['de', 'fghi']]
    ssi2 = [[], [], ['xyz']]

    rso, sso = p.opStringSS(ssi1, ssi2)
    test(sso.length == 5)
    test(sso[0].length == 1)
    test(sso[0][0] == "abc")
    test(sso[1].length == 2)
    test(sso[1][0] == "de")
    test(sso[1][1] == "fghi")
    test(sso[2].length == 0)
    test(sso[3].length == 0)
    test(sso[4].length == 1)
    test(sso[4][0] == "xyz")
    test(rso.length == 3)
    test(rso[0].length == 1)
    test(rso[0][0] == "xyz")
    test(rso[1].length == 0)
    test(rso[2].length == 0)

    #
    # opStringSSS
    #
    sssi1 = [[['abc', 'de'], ['xyz']], [['hello']]]
    sssi2 = [[['', ''], ['abcd']], [['']], []]

    rsso, ssso = p.opStringSSS(sssi1, sssi2)
    test(ssso.length == 5)
    test(ssso[0].length == 2)
    test(ssso[0][0].length == 2)
    test(ssso[0][1].length == 1)
    test(ssso[1].length == 1)
    test(ssso[1][0].length == 1)
    test(ssso[2].length == 2)
    test(ssso[2][0].length == 2)
    test(ssso[2][1].length == 1)
    test(ssso[3].length == 1)
    test(ssso[3][0].length == 1)
    test(ssso[4].length == 0)
    test(ssso[0][0][0] == "abc")
    test(ssso[0][0][1] == "de")
    test(ssso[0][1][0] == "xyz")
    test(ssso[1][0][0] == "hello")
    test(ssso[2][0][0] == "")
    test(ssso[2][0][1] == "")
    test(ssso[2][1][0] == "abcd")
    test(ssso[3][0][0] == "")

    test(rsso.length == 3)
    test(rsso[0].length == 0)
    test(rsso[1].length == 1)
    test(rsso[1][0].length == 1)
    test(rsso[2].length == 2)
    test(rsso[2][0].length == 2)
    test(rsso[2][1].length == 1)
    test(rsso[1][0][0] == "")
    test(rsso[2][0][0] == "")
    test(rsso[2][0][1] == "")
    test(rsso[2][1][0] == "abcd")

    #
    # opByteBoolD
    #
    di1 = {10=>true, 100=>false}
    di2 = {10=>true, 11=>false, 101=>true}

    ro, d = p.opByteBoolD(di1, di2)

    test(d == di1)
    test(ro.length == 4)
    test(ro[10])
    test(!ro[11])
    test(!ro[100])
    test(ro[101])

    #
    # opShortIntD
    #
    di1 = {110=>-1, 1100=>123123}
    di2 = {110=>-1, 111=>-100, 1101=>0}

    ro, d = p.opShortIntD(di1, di2)

    test(d == di1)
    test(ro.length == 4)
    test(ro[110] == -1)
    test(ro[111] == -100)
    test(ro[1100] == 123123)
    test(ro[1101] == 0)

    #
    # opLongFloatD
    #
    di1 = {999999110=>-1.1, 999999111=>123123.2}
    di2 = {999999110=>-1.1, 999999120=>-100.4, 999999130=>0.5}

    ro, d = p.opLongFloatD(di1, di2)

    for k in d.keys
        test((d[k] - di1[k]).abs < 0.01)
    end
    test(ro.length == 4)
    test(ro[999999110] - -1.1 < 0.01)
    test(ro[999999120] - -100.4 < 0.01)
    test(ro[999999111] - 123123.2 < 0.01)
    test(ro[999999130] - 0.5 < 0.01)

    #
    # opStringStringD
    #
    di1 = {'foo'=>'abc -1.1', 'bar'=>'abc 123123.2'}
    di2 = {'foo'=>'abc -1.1', 'FOO'=>'abc -100.4', 'BAR'=>'abc 0.5'}

    ro, d = p.opStringStringD(di1, di2)

    test(d == di1)
    test(ro.length == 4)
    test(ro["foo"] == "abc -1.1")
    test(ro["FOO"] == "abc -100.4")
    test(ro["bar"] == "abc 123123.2")
    test(ro["BAR"] == "abc 0.5")

    #
    # opStringMyEnumD
    #
    di1 = {'abc'=>Test::MyEnum::Enum1, ''=>Test::MyEnum::Enum2}
    di2 = {'abc'=>Test::MyEnum::Enum1, 'qwerty'=>Test::MyEnum::Enum3, 'Hello!!'=>Test::MyEnum::Enum2}

    ro, d = p.opStringMyEnumD(di1, di2)

    test(d == di1)
    test(ro.length == 4)
    test(ro["abc"] == Test::MyEnum::Enum1)
    test(ro["qwerty"] == Test::MyEnum::Enum3)
    test(ro[""] == Test::MyEnum::Enum2)
    test(ro["Hello!!"] == Test::MyEnum::Enum2)

    #
    # opMyEnumStringD
    #
    di1 = {Test::MyEnum::Enum1=>'abc'}
    di2 = {Test::MyEnum::Enum2=>'Hello!!', Test::MyEnum::Enum3=>'qwerty'}

    ro, d = p.opMyEnumStringD(di1, di2)

    test(d == di1)
    test(ro.length == 3)
    test(ro[Test::MyEnum::Enum1] == "abc")
    test(ro[Test::MyEnum::Enum2] == "Hello!!")
    test(ro[Test::MyEnum::Enum3] == "qwerty")

    #
    # opMyStructMyEnumD
    #
    s11 = Test::MyStruct.new
    s11.i = 1
    s11.j = 1
    s12 = Test::MyStruct.new
    s12.i = 1
    s12.j = 2
    s22 = Test::MyStruct.new
    s22.i = 2
    s22.j = 2
    s23 = Test::MyStruct.new
    s23.i = 2
    s23.j = 3
    di1 = {s11=>Test::MyEnum::Enum1, s12=>Test::MyEnum::Enum2}
    di2 = {s11=>Test::MyEnum::Enum1, s22=>Test::MyEnum::Enum3, s23=>Test::MyEnum::Enum2}

    ro, d = p.opMyStructMyEnumD(di1, di2)

    test(d == di1)
    test(ro.length == 4)
    test(ro[s11] == Test::MyEnum::Enum1)
    test(ro[s12] == Test::MyEnum::Enum2)
    test(ro[s22] == Test::MyEnum::Enum3)
    test(ro[s23] == Test::MyEnum::Enum2)

    #
    # opByteBoolDS
    #
    # di1 = {10=>true, 100=>false}
    # di2 = {10=>true, 11=>false, 101=>true}

    dsi1 = [{ 10=>true, 100=>false }, { 10=>true, 11=>false, 101=>true }]
    dsi2 = [{ 100=>false, 101=>false }]

    ro, d = p.opByteBoolDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 3)
    test(ro[0][10])
    test(!ro[0][11])
    test(ro[0][101])
    test(ro[1].length == 2)
    test(ro[1][10])
    test(!ro[1][100])
    test(d.length == 3)
    test(d[0].length == 2)
    test(!d[0][100])
    test(!d[0][101])
    test(d[1].length == 2)
    test(d[1][10])
    test(!d[1][100])
    test(d[2].length == 3)
    test(d[2][10])
    test(!d[2][11])
    test(d[2][101])

    #
    # opShortIntDS
    #
    dsi1 = [{ 110=>-1, 1100=>123123 }, { 110=>-1, 111=>-100, 1101=>0 }]
    dsi2 = [{ 100=>-1001 }]

    ro, d = p.opShortIntDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 3)
    test(ro[0][110] == -1)
    test(ro[0][111] == -100)
    test(ro[0][1101] == 0)
    test(ro[1].length == 2)
    test(ro[1][110] == -1)
    test(ro[1][1100] == 123123)

    test(d.length == 3)
    test(d[0].length == 1)
    test(d[0][100] == -1001)
    test(d[1].length == 2)
    test(d[1][110] == -1)
    test(d[1][1100] == 123123)
    test(d[2].length == 3)
    test(d[2][110] == -1)
    test(d[2][111] == -100)
    test(d[2][1101] == 0)

    #
    # opLongFloatDS
    #
    dsi1 = [{ 999999110=>-1.1, 999999111=>123123.2 }, { 999999110=>-1.1, 999999120=>-100.4, 999999130=>0.5 }]
    dsi2 = [{ 999999140=>3.14 }]

    ro, d = p.opLongFloatDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 3)
    test(ro[0][999999110] - -1.1 < 0.01)
    test(ro[0][999999120] - -100.4 < 0.01)
    test(ro[0][999999130] - 0.5 < 0.01)
    test(ro[1].length == 2)
    test(ro[1][999999110] - -1.1 < 0.01)
    test(ro[1][999999111] - 123123.2 < 0.01)
    test(d.length == 3)
    test(d[0].length == 1)
    test(d[0][999999140] - 3.14 < 0.01)
    test(d[1].length == 2)
    test(d[1][999999110] - -1.1 < 0.01)
    test(d[1][999999111] - 123123.2 < 0.01)
    test(d[2].length == 3)
    test(d[2][999999110] - -1.1 < 0.01)
    test(d[2][999999120] - -100.4 < 0.01)
    test(d[2][999999130] - 0.5 < 0.01)

    #
    # opStringStringDS
    #

    dsi1 = [{ "foo"=>"abc -1.1", "bar"=>"abc 123123.2" }, { "foo"=>"abc -1.1", "FOO"=>"abc -100.4", "BAR"=>"abc 0.5" }]
    dsi2 = [{ "f00"=>"ABC -3.14" }]

    ro, d = p.opStringStringDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 3)
    test(ro[0]["foo"] == "abc -1.1")
    test(ro[0]["FOO"] == "abc -100.4")
    test(ro[0]["BAR"] == "abc 0.5")
    test(ro[1].length == 2)
    test(ro[1]["foo"] == "abc -1.1")
    test(ro[1]["bar"] == "abc 123123.2")

    test(d.length == 3)
    test(d[0].length == 1)
    test(d[0]["f00"] == "ABC -3.14")
    test(d[1].length == 2)
    test(d[1]["foo"] == "abc -1.1")
    test(d[1]["bar"] == "abc 123123.2")
    test(d[2].length == 3)
    test(d[2]["foo"] == "abc -1.1")
    test(d[2]["FOO"] == "abc -100.4")
    test(d[2]["BAR"] == "abc 0.5")

    #
    # opStringMyEnumDS
    #
    dsi1 = [
            { "abc"=>Test::MyEnum::Enum1, ""=>Test::MyEnum::Enum2 },
            { "abc"=>Test::MyEnum::Enum1, "qwerty"=>Test::MyEnum::Enum3, "Hello!!"=>Test::MyEnum::Enum2 }
           ]

    dsi2 = [{ "Goodbye"=>Test::MyEnum::Enum1 }]

    ro, d = p.opStringMyEnumDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 3)
    test(ro[0]["abc"] == Test::MyEnum::Enum1)
    test(ro[0]["qwerty"] == Test::MyEnum::Enum3)
    test(ro[0]["Hello!!"] == Test::MyEnum::Enum2)
    test(ro[1].length == 2)
    test(ro[1]["abc"] == Test::MyEnum::Enum1)
    test(ro[1][""] == Test::MyEnum::Enum2)

    test(d.length == 3)
    test(d[0].length == 1)
    test(d[0]["Goodbye"] == Test::MyEnum::Enum1)
    test(d[1].length == 2)
    test(d[1]["abc"] == Test::MyEnum::Enum1)
    test(d[1][""] == Test::MyEnum::Enum2)
    test(d[2].length == 3)
    test(d[2]["abc"] == Test::MyEnum::Enum1)
    test(d[2]["qwerty"] == Test::MyEnum::Enum3)
    test(d[2]["Hello!!"] == Test::MyEnum::Enum2)

    #
    # opMyEnumStringDS
    #
    dsi1 = [{ Test::MyEnum::Enum1=>'abc' }, { Test::MyEnum::Enum2=>'Hello!!', Test::MyEnum::Enum3=>'qwerty'}]
    dsi2 = [{ Test::MyEnum::Enum1=>'Goodbye' }]

    ro, d = p.opMyEnumStringDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 2)
    test(ro[0][Test::MyEnum::Enum2] == "Hello!!")
    test(ro[0][Test::MyEnum::Enum3] == "qwerty")
    test(ro[1].length == 1)
    test(ro[1][Test::MyEnum::Enum1] == "abc")

    test(d.length == 3)
    test(d[0].length == 1)
    test(d[0][Test::MyEnum::Enum1] == "Goodbye")
    test(d[1].length == 1)
    test(d[1][Test::MyEnum::Enum1] == "abc")
    test(d[2].length == 2)
    test(d[2][Test::MyEnum::Enum2] == "Hello!!")
    test(d[2][Test::MyEnum::Enum3] == "qwerty")

    #
    # opMyStructMyEnumDS
    #
    s11 = Test::MyStruct.new
    s11.i = 1
    s11.j = 1
    s12 = Test::MyStruct.new
    s12.i = 1
    s12.j = 2
    s22 = Test::MyStruct.new
    s22.i = 2
    s22.j = 2
    s23 = Test::MyStruct.new
    s23.i = 2
    s23.j = 3

    dsi1 = [
            { s11=>Test::MyEnum::Enum1, s12=>Test::MyEnum::Enum2 },
            { s11=>Test::MyEnum::Enum1, s22=>Test::MyEnum::Enum3, s23=>Test::MyEnum::Enum2 }
           ]
    dsi2 = [{ s23=>Test::MyEnum::Enum3 }]

    ro, d = p.opMyStructMyEnumDS(dsi1, dsi2)

    test(ro.length == 2)
    test(ro[0].length == 3)
    test(ro[0][s11] == Test::MyEnum::Enum1)
    test(ro[0][s22] == Test::MyEnum::Enum3)
    test(ro[0][s23] == Test::MyEnum::Enum2)
    test(ro[1].length == 2)
    test(ro[1][s11] == Test::MyEnum::Enum1)
    test(ro[1][s12] == Test::MyEnum::Enum2)

    test(d.length == 3)
    test(d[0].length == 1)
    test(d[0][s23] == Test::MyEnum::Enum3)
    test(d[1].length == 2)
    test(d[1][s11] == Test::MyEnum::Enum1)
    test(d[1][s12] == Test::MyEnum::Enum2)
    test(d[2].length == 3)
    test(d[2][s11] == Test::MyEnum::Enum1)
    test(d[2][s22] == Test::MyEnum::Enum3)
    test(d[2][s23] == Test::MyEnum::Enum2)

    #
    # opByteByteSD
    #
    sdi1 = { 0x01=>[0x01, 0x11], 0x22=>[0x12] }
    sdi2 = { 0xf1=>[0xf2, 0xf3] }

    ro, d = p.opByteByteSD(sdi1, sdi2)

    test(d.length == 1)
    test(d[0xf1].length == 2)
    test(d[0xf1].unpack("C*") == [0xf2, 0xf3])
    test(ro.length == 3)
    test(ro[0x01].length == 2)
    test(ro[0x01].unpack("C*") == [0x01, 0x11])
    test(ro[0x22].length == 1)
    test(ro[0x22].unpack("C*") == [0x12])
    test(ro[0xf1].length == 2)
    test(ro[0xf1].unpack("C*") == [0xf2, 0xf3])

    #
    # opBoolBoolSD
    #
    sdi1 = { false=>[true, false], true=>[false, true, true] }
    sdi2 = { false=>[true, false] }

    ro, d = p.opBoolBoolSD(sdi1, sdi2)

    test(d.length == 1)
    test(d[false].length == 2)
    test(d[false][0])
    test(!d[false][1])
    test(ro.length == 2)
    test(ro[false].length == 2)
    test(ro[false][0])
    test(!ro[false][1])
    test(ro[true].length == 3)
    test(!ro[true][0])
    test(ro[true][1])
    test(ro[true][2])

    #
    # opShortShortSD
    #
    sdi1 = { 1=>[1, 2, 3], 2=>[4, 5] }
    sdi2 = { 4=>[6, 7] }

    ro, d = p.opShortShortSD(sdi1, sdi2)

    test(d.length == 1)
    test(d[4].length == 2)
    test(d[4][0] == 6)
    test(d[4][1] == 7)
    test(ro.length == 3)
    test(ro[1].length == 3)
    test(ro[1][0] == 1)
    test(ro[1][1] == 2)
    test(ro[1][2] == 3)
    test(ro[2].length == 2)
    test(ro[2][0] == 4)
    test(ro[2][1] == 5)
    test(ro[4].length == 2)
    test(ro[4][0] == 6)
    test(ro[4][1] == 7)

    #
    # opIntIntSD
    #
    sdi1 = { 100=>[100, 200, 300], 200=>[400, 500] }
    sdi2 = { 400=>[600, 700] }

    ro, d = p.opIntIntSD(sdi1, sdi2)

    test(d.length == 1)
    test(d[400].length == 2)
    test(d[400][0] == 600)
    test(d[400][1] == 700)
    test(ro.length == 3)
    test(ro[100].length == 3)
    test(ro[100][0] == 100)
    test(ro[100][1] == 200)
    test(ro[100][2] == 300)
    test(ro[200].length == 2)
    test(ro[200][0] == 400)
    test(ro[200][1] == 500)
    test(ro[400].length == 2)
    test(ro[400][0] == 600)
    test(ro[400][1] == 700)

    #
    # opLongLongSD
    #
    sdi1 = { 999999990=>[999999110, 999999111, 999999110], 999999991=>[999999120, 999999130] }
    sdi2 = { 999999992=>[999999110, 999999120] }

    ro, d = p.opLongLongSD(sdi1, sdi2)

    test(d.length == 1)
    test(d[999999992].length == 2)
    test(d[999999992][0] == 999999110)
    test(d[999999992][1] == 999999120)
    test(ro.length == 3)
    test(ro[999999990].length == 3)
    test(ro[999999990][0] == 999999110)
    test(ro[999999990][1] == 999999111)
    test(ro[999999990][2] == 999999110)
    test(ro[999999991].length == 2)
    test(ro[999999991][0] == 999999120)
    test(ro[999999991][1] == 999999130)
    test(ro[999999992].length == 2)
    test(ro[999999992][0] == 999999110)
    test(ro[999999992][1] == 999999120)

    #
    # opStringFloatSD
    #
    sdi1 = { "abc"=>[-1.1, 123123.2, 100.0], "ABC"=>[42.24, -1.61] }
    sdi2 = { "aBc"=>[-3.14, 3.14] }

    ro, d = p.opStringFloatSD(sdi1, sdi2)

    test(d.length == 1)
    test(d["aBc"].length == 2)
    test(d["aBc"][0] - -3.14 < 0.01)
    test(d["aBc"][1] - 3.14 < 0.01)

    test(ro.length == 3)
    test(ro["abc"].length == 3)
    test(ro["abc"][0] - -1.1 < 0.01)
    test(ro["abc"][1] - 123123.2 < 0.01)
    test(ro["abc"][2] - 100.0 < 0.01)
    test(ro["ABC"].length == 2)
    test(ro["ABC"][0] - 42.24 < 0.01)
    test(ro["ABC"][1] - -1.61 < 0.01)
    test(ro["aBc"].length == 2)
    test(ro["aBc"][0] - -3.14 < 0.01)
    test(ro["aBc"][1] - 3.14 < 0.01)

    #
    # opStringDoubleSD
    #
    sdi1 = { "Hello!!"=>[1.1E10, 1.2E10, 1.3E10], "Goodbye"=>[1.4E10, 1.5E10] }
    sdi2 = { ""=>[1.6E10, 1.7E10] }

    ro, d = p.opStringDoubleSD(sdi1, sdi2);

    test(d.length == 1)
    test(d[""].length == 2)
    test(d[""][0] == 1.6E10)
    test(d[""][1] == 1.7E10)
    test(ro.length == 3)
    test(ro["Hello!!"].length == 3)
    test(ro["Hello!!"][0] == 1.1E10)
    test(ro["Hello!!"][1] == 1.2E10)
    test(ro["Hello!!"][2] == 1.3E10)
    test(ro["Goodbye"].length == 2)
    test(ro["Goodbye"][0] == 1.4E10)
    test(ro["Goodbye"][1] == 1.5E10)
    test(ro[""].length == 2)
    test(ro[""][0] == 1.6E10)
    test(ro[""][1] == 1.7E10)

    #
    # opStringStringSD
    #
    sdi1 = { "abc"=>["abc", "de", "fghi"] , "def"=>["xyz", "or"] }
    sdi2 = { "ghi"=>["and", "xor"] }

    ro, d = p.opStringStringSD(sdi1, sdi2)

    test(d.length == 1)
    test(d["ghi"].length == 2)
    test(d["ghi"][0] == "and")
    test(d["ghi"][1] == "xor")
    test(ro.length == 3)
    test(ro["abc"].length == 3)
    test(ro["abc"][0] == "abc")
    test(ro["abc"][1] == "de")
    test(ro["abc"][2] == "fghi")
    test(ro["def"].length == 2)
    test(ro["def"][0] == "xyz")
    test(ro["def"][1] == "or")
    test(ro["ghi"].length == 2)
    test(ro["ghi"][0] == "and")
    test(ro["ghi"][1] == "xor")

    #
    # opMyEnumMyEnumSD
    #
    sdi1 = {
            Test::MyEnum::Enum3=>[Test::MyEnum::Enum1, Test::MyEnum::Enum1, Test::MyEnum::Enum2],
            Test::MyEnum::Enum2=>[Test::MyEnum::Enum1, Test::MyEnum::Enum2]
           }
    sdi2 = { Test::MyEnum::Enum1=>[Test::MyEnum::Enum3, Test::MyEnum::Enum3] }

    ro, d = p.opMyEnumMyEnumSD(sdi1, sdi2)

    test(d.length == 1)
    test(d[Test::MyEnum::Enum1].length == 2)
    test(d[Test::MyEnum::Enum1][0] == Test::MyEnum::Enum3)
    test(d[Test::MyEnum::Enum1][1] == Test::MyEnum::Enum3)
    test(ro.length == 3)
    test(ro[Test::MyEnum::Enum3].length == 3)
    test(ro[Test::MyEnum::Enum3][0] == Test::MyEnum::Enum1)
    test(ro[Test::MyEnum::Enum3][1] == Test::MyEnum::Enum1)
    test(ro[Test::MyEnum::Enum3][2] == Test::MyEnum::Enum2)
    test(ro[Test::MyEnum::Enum2].length == 2)
    test(ro[Test::MyEnum::Enum2][0] == Test::MyEnum::Enum1)
    test(ro[Test::MyEnum::Enum2][1] == Test::MyEnum::Enum2)
    test(ro[Test::MyEnum::Enum1].length == 2)
    test(ro[Test::MyEnum::Enum1][0] == Test::MyEnum::Enum3)
    test(ro[Test::MyEnum::Enum1][1] == Test::MyEnum::Enum3)

    #
    # opIntS
    #
    lengths = [ 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 ]
    for l in lengths
        s = []
        for i in (0...l)
            s.push(i)
        end
        r = p.opIntS(s)
        test(r.length == l)
        for j in (0...r.length)
            test(r[j] == -j)
        end
    end

    #
    # opContext
    #
    ctx = {'one'=>'ONE', 'two'=>'TWO', 'three'=>'THREE'}

    r = p.opContext()
    test(p.ice_getContext().length == 0)
    test(r != ctx)

    r = p.opContext(ctx)
    test(p.ice_getContext().length == 0)
    test(r == ctx)

    p2 = Test::MyClassPrx::checkedCast(p.ice_context(ctx))
    test(p2.ice_getContext() == ctx)
    r = p2.opContext()
    test(r == ctx)
    r = p2.opContext(ctx)
    test(r == ctx)

    #
    # opIdempotent
    #
    p.opIdempotent

    #
    # opNonmutating
    #
    p.opNonmutating

    test(p.opByte1(0xFF) == 0xFF)
    test(p.opShort1(0x7FFF) == 0x7FFF)
    test(p.opInt1(0x7FFFFFFF) == 0x7FFFFFFF)
    test(p.opLong1(0x7FFFFFFFFFFFFFFF) == 0x7FFFFFFFFFFFFFFF)
    test(p.opFloat1(1.0) == 1.0)
    test(p.opDouble1(1.0) == 1.0)
    test(p.opString1("opString1") == "opString1")
    test(p.opStringS1(nil).length == 0)
    test(p.opByteBoolD1(nil).length == 0)
    test(p.opStringS2(nil).length == 0)
    test(p.opByteBoolD2(nil).length == 0)

    d = Test::MyDerivedClassPrx::uncheckedCast(p)
    s = Test::MyStruct1.new
    s.tesT = "Test.MyStruct1.s"
    s.myClass = nil
    s.myStruct1 = "Test.MyStruct1.myStruct1"
    s = d.opMyStruct1(s)
    test(s.tesT == "Test.MyStruct1.s")
    test(s.myClass == nil)
    test(s.myStruct1 == "Test.MyStruct1.myStruct1")
    c = Test::MyClass1.new
    c.tesT = "Test.MyClass1.testT"
    c.myClass = nil
    c.myClass1 = "Test.MyClass1.myClass1"
    c = d.opMyClass1(c)
    test(c.tesT == "Test.MyClass1.testT")
    test(c.myClass == nil)
    test(c.myClass1 == "Test.MyClass1.myClass1")

    p1 = p.opMStruct1()
    p1.e = Test::MyEnum::Enum3
    (p3, p2) = p.opMStruct2(p1)
    test(p2 == p1 && p3 == p1)

    p.opMSeq1();
    p1 = ["test"]
    (p3, p2) = p.opMSeq2(p1)
    test(p2[0] == "test" && p3[0] == "test");

    p.opMDict1();

    p1 = { "test" => "test" }
    (p3, p2) = p.opMDict2(p1)
    test(p3["test"] == "test" && p2["test"] == "test")

    #
    # Test implicit context propagation
    #
    impls = [ 'Shared', 'PerThread' ]
    for i in impls
        initData = Ice::InitializationData.new
        initData.properties = communicator.getProperties().clone()
        initData.properties.setProperty('Ice.ImplicitContext', i)
        ic = Ice::initialize(initData)

        ctx = {'one'=>'ONE', 'two'=>'TWO', 'three'=>'THREE'}

        p = Test::MyClassPrx::uncheckedCast(ic.stringToProxy('test:default -p 12010'))

        ic.getImplicitContext().setContext(ctx)
        test(ic.getImplicitContext().getContext() == ctx)
        test(p.opContext() == ctx)

        test(ic.getImplicitContext().containsKey('zero') == false);
        r = ic.getImplicitContext().put('zero', 'ZERO');
        test(r == '');
        test(ic.getImplicitContext().containsKey('zero') == true);
        test(ic.getImplicitContext().get('zero') == 'ZERO');

        ctx = ic.getImplicitContext().getContext()
        test(p.opContext() == ctx)

        prxContext = {'one'=>'UN', 'four'=>'QUATRE'}

        combined = ctx.clone()
        combined.update(prxContext)
        test(combined['one'] == 'UN')

        p = Test::MyClassPrx::uncheckedCast(p.ice_context(prxContext))
        ic.getImplicitContext().setContext({})
        test(p.opContext() == prxContext)

        ic.getImplicitContext().setContext(ctx)
        test(p.opContext() == combined)

        test(ic.getImplicitContext().remove('one') == 'ONE');

        ic.destroy()
    end
end
