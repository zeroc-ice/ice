#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def twoways(communicator, p)
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
    test(c1.ice_getIdentity() == communicator.stringToIdentity("test"))
    test(c2.ice_getIdentity() == communicator.stringToIdentity("noSuchIdentity"))
    test(r.ice_getIdentity() == communicator.stringToIdentity("test"))
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

    #
    # opByteS
    #
    bsi1 = [0x01, 0x11, 0x12, 0x22]
    bsi2 = [0xf1, 0xf2, 0xf3, 0xf4]

    rso, bso = p.opByteS(bsi1, bsi2)
    test(bso.length == 4)
    test(bso[0] == 0x22)
    test(bso[1] == 0x12)
    test(bso[2] == 0x11)
    test(bso[3] == 0x01)
    test(rso.length == 8)
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
    test(bso[0][0] == 0xff)
    test(bso[1].length == 3)
    test(bso[1][0] == 0x01)
    test(bso[1][1] == 0x11)
    test(bso[1][2] == 0x12)
    test(rso.length == 4)
    test(rso[0].length == 3)
    test(rso[0][0] == 0x01)
    test(rso[0][1] == 0x11)
    test(rso[0][2] == 0x12)
    test(rso[1].length == 1)
    test(rso[1][0] == 0xff)
    test(rso[2].length == 1)
    test(rso[2][0] == 0x0e)
    test(rso[3].length == 2)
    test(rso[3][0] == 0xf2)
    test(rso[3][1] == 0xf1)

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
    # Test that default context is obtained correctly from communicator.
    #
# DEPRECATED
#    dflt = {'a'=>'b'}
#    communicator.setDefaultContext(dflt)
#    test(p.opContext() != dflt)
#
#    p2 = Test::MyClassPrx::uncheckedCast(p.ice_context({}))
#    test(p2.opContext().length == 0)
#
#    p2 = Test::MyClassPrx::uncheckedCast(p.ice_defaultContext())
#    test(p2.opContext() == dflt)
#
#    communicator.setDefaultContext({})
#    test(p2.opContext().length > 0)
#
#    communicator.setDefaultContext(dflt)
#    c = Test::MyClassPrx::checkedCast(communicator.stringToProxy("test:default -p 12010 -t 10000"))
#    test(c.opContext() == dflt)
#
#    dflt['a'] = 'c'
#    c2 = Test::MyClassPrx::uncheckedCast(c.ice_context(dflt))
#    test(c2.opContext()['a'] == 'c')
#
#    dflt = {}
#    c3 = Test::MyClassPrx::uncheckedCast(c2.ice_context(dflt))
#    tmp = c3.opContext()
#    test(!tmp.has_key?('a'))
#
#    c4 = Test::MyClassPrx::uncheckedCast(c2.ice_defaultContext())
#    test(c4.opContext()['a'] == 'b')
#
#    dflt['a'] = 'd'
#    communicator.setDefaultContext(dflt)
#
#    c5 = Test::MyClassPrx::uncheckedCast(c.ice_defaultContext())
#    test(c5.opContext()['a'] == 'd')
#
#    communicator.setDefaultContext({})

    #
    # Test implicit context propagation
    #
    impls = [ 'Shared', 'PerThread' ]
    for i in impls
        initData = Ice::InitializationData.new
        initData.properties = communicator.getProperties().clone()
        initData.properties.setProperty('Ice.ImplicitContext', i)
        ic = Ice.initialize(initData)
        
        ctx = {'one'=>'ONE', 'two'=>'TWO', 'three'=>'THREE'}
        
        p = Test::MyClassPrx::uncheckedCast(ic.stringToProxy('test:default -p 12010 -t 10000'))
        
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
