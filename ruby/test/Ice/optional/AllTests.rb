# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(helper, communicator)
    print "testing stringToProxy... "
    STDOUT.flush
    ref = "initial:#{helper.getTestEndpoint()}"
    base = communicator.stringToProxy(ref)
    test(base)
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    initial = Test::InitialPrx::checkedCast(base)
    test(initial)
    test(initial == base)
    puts "ok"

    print "testing optional data members... "
    STDOUT.flush

    oo1 = Test::OneOptional.new
    test(oo1.a == Ice::Unset)
    oo1.a = 15

    oo2 = Test::OneOptional.new(16)
    test(oo2.a == 16)

    mo1 = Test::MultiOptional.new()
    test(mo1.a == Ice::Unset)
    test(mo1.b == Ice::Unset)
    test(mo1.c == Ice::Unset)
    test(mo1.d == Ice::Unset)
    test(mo1.e == Ice::Unset)
    test(mo1.f == Ice::Unset)
    test(mo1.g == Ice::Unset)
    test(mo1.h == Ice::Unset)
    test(mo1.i == Ice::Unset)
    test(mo1.j == Ice::Unset)
    test(mo1.k == Ice::Unset)
    test(mo1.bs == Ice::Unset)
    test(mo1.ss == Ice::Unset)
    test(mo1.iid == Ice::Unset)
    test(mo1.sid == Ice::Unset)
    test(mo1.fs == Ice::Unset)
    test(mo1.vs == Ice::Unset)

    test(mo1.shs == Ice::Unset)
    test(mo1.es == Ice::Unset)
    test(mo1.fss == Ice::Unset)
    test(mo1.vss == Ice::Unset)
    test(mo1.oos == Ice::Unset)
    test(mo1.oops == Ice::Unset)

    test(mo1.ied == Ice::Unset)
    test(mo1.ifsd == Ice::Unset)
    test(mo1.ivsd == Ice::Unset)
    test(mo1.iood == Ice::Unset)
    test(mo1.ioopd == Ice::Unset)

    test(mo1.bos == Ice::Unset)

    ss = Test::SmallStruct.new()
    fs = Test::FixedStruct.new(78)
    vs = Test::VarStruct.new("hello")
    mo1 = Test::MultiOptional.new(15, true, 19, 78, 99, 5.5, 1.0, "test", Test::MyEnum::MyEnumMember, \
                                  communicator.stringToProxy("test"), \
                                  nil, [5], ["test", "test2"], {4=>3}, {"test"=>10}, fs, vs, [1], \
                                  [Test::MyEnum::MyEnumMember, Test::MyEnum::MyEnumMember], \
                                  [ fs ], [ vs ], [ oo1 ], \
                                  [ communicator.stringToProxy("test") ], \
                                  {4=> Test::MyEnum::MyEnumMember}, {4=>fs}, {5=>vs}, {5=>Test::OneOptional.new(15)}, \
                                  {5=> communicator.stringToProxy("test")}, \
                                  [false, true, false])

    test(mo1.a == 15)
    test(mo1.b == true)
    test(mo1.c == 19)
    test(mo1.d == 78)
    test(mo1.e == 99)
    test(mo1.f == 5.5)
    test(mo1.g == 1.0)
    test(mo1.h == "test")
    test(mo1.i == Test::MyEnum::MyEnumMember)
    test(mo1.j == communicator.stringToProxy("test"))
    test(mo1.k == nil)
    test(mo1.bs == [5])
    test(mo1.ss == ["test", "test2"])
    test(mo1.iid[4] == 3)
    test(mo1.sid["test"] == 10)
    test(mo1.fs == Test::FixedStruct.new(78))
    test(mo1.vs == Test::VarStruct.new("hello"))

    test(mo1.shs[0] == 1)
    test(mo1.es[0] == Test::MyEnum::MyEnumMember && mo1.es[1] == Test::MyEnum::MyEnumMember)
    test(mo1.fss[0] == Test::FixedStruct.new(78))
    test(mo1.vss[0] == Test::VarStruct.new("hello"))
    test(mo1.oos[0] == oo1)
    test(mo1.oops[0] == communicator.stringToProxy("test"))

    test(mo1.ied[4] == Test::MyEnum::MyEnumMember)
    test(mo1.ifsd[4] == Test::FixedStruct.new(78))
    test(mo1.ivsd[5] == Test::VarStruct.new("hello"))
    test(mo1.iood[5].a == 15)
    test(mo1.ioopd[5] == communicator.stringToProxy("test"))

    test(mo1.bos == [false, true, false])

    #
    # Test generated struct and classes compare with Ice::Unset
    #
    test(ss != Ice::Unset)
    test(fs != Ice::Unset)
    test(vs != Ice::Unset)
    test(mo1 != Ice::Unset)

    puts "ok"

    print "testing marshaling... "
    STDOUT.flush

    oo4 = initial.pingPong(Test::OneOptional.new)
    test(oo4.a == Ice::Unset)

    oo5 = initial.pingPong(oo1)
    test(oo1.a == oo5.a)

    mo4 = initial.pingPong(Test::MultiOptional.new)
    test(mo4.a == Ice::Unset)
    test(mo4.b == Ice::Unset)
    test(mo4.c == Ice::Unset)
    test(mo4.d == Ice::Unset)
    test(mo4.e == Ice::Unset)
    test(mo4.f == Ice::Unset)
    test(mo4.g == Ice::Unset)
    test(mo4.h == Ice::Unset)
    test(mo4.i == Ice::Unset)
    test(mo4.j == Ice::Unset)
    test(mo4.k == Ice::Unset)
    test(mo4.bs == Ice::Unset)
    test(mo4.ss == Ice::Unset)
    test(mo4.iid == Ice::Unset)
    test(mo4.sid == Ice::Unset)
    test(mo4.fs == Ice::Unset)
    test(mo4.vs == Ice::Unset)

    test(mo4.shs == Ice::Unset)
    test(mo4.es == Ice::Unset)
    test(mo4.fss == Ice::Unset)
    test(mo4.vss == Ice::Unset)
    test(mo4.oos == Ice::Unset)
    test(mo4.oops == Ice::Unset)

    test(mo4.ied == Ice::Unset)
    test(mo4.ifsd == Ice::Unset)
    test(mo4.ivsd == Ice::Unset)
    test(mo4.iood == Ice::Unset)
    test(mo4.ioopd == Ice::Unset)

    test(mo4.bos == Ice::Unset)

    mo5 = initial.pingPong(mo1)
    test(mo5.a == mo1.a)
    test(mo5.b == mo1.b)
    test(mo5.c == mo1.c)
    test(mo5.d == mo1.d)
    test(mo5.e == mo1.e)
    test(mo5.f == mo1.f)
    test(mo5.g == mo1.g)
    test(mo5.h == mo1.h)
    test(mo5.i == mo1.i)
    test(mo5.j == mo1.j)
    test(mo5.k == nil)
    test(mo5.bs.unpack("C*") == [0x05])
    test(mo5.ss == mo1.ss)
    test(mo5.iid[4] == 3)
    test(mo5.sid["test"] == 10)
    test(mo5.fs == mo1.fs)
    test(mo5.vs == mo1.vs)
    test(mo5.shs == mo1.shs)
    test(mo5.es[0] == Test::MyEnum::MyEnumMember && mo1.es[1] == Test::MyEnum::MyEnumMember)
    test(mo5.fss[0] == Test::FixedStruct.new(78))
    test(mo5.vss[0] == Test::VarStruct.new("hello"))
    test(mo5.oos[0].a == 15)
    test(mo5.oops[0] == communicator.stringToProxy("test"))

    test(mo5.ied[4] == Test::MyEnum::MyEnumMember)
    test(mo5.ifsd[4] == Test::FixedStruct.new(78))
    test(mo5.ivsd[5] == Test::VarStruct.new("hello"))
    test(mo5.iood[5].a == 15)
    test(mo5.ioopd[5] == communicator.stringToProxy("test"))

    test(mo5.bos == mo1.bos)

    # Clear the first half of the optional members
    mo6 = Test::MultiOptional.new
    mo6.b = mo5.b
    mo6.d = mo5.d
    mo6.f = mo5.f
    mo6.h = mo5.h
    mo6.j = mo5.j
    mo6.bs = mo5.bs
    mo6.iid = mo5.iid
    mo6.fs = mo5.fs
    mo6.shs = mo5.shs
    mo6.fss = mo5.fss
    mo6.oos = mo5.oos
    mo6.ifsd = mo5.ifsd
    mo6.iood = mo5.iood
    mo6.bos = mo5.bos

    mo7 = initial.pingPong(mo6)
    test(mo7.a == Ice::Unset)
    test(mo7.b == mo1.b)
    test(mo7.c == Ice::Unset)
    test(mo7.d == mo1.d)
    test(mo7.e == Ice::Unset)
    test(mo7.f == mo1.f)
    test(mo7.g == Ice::Unset)
    test(mo7.h == mo1.h)
    test(mo7.i == Ice::Unset)
    test(mo7.j == mo1.j)
    test(mo7.k == Ice::Unset)
    test(mo7.bs.unpack("C*") == [0x05])
    test(mo7.ss == Ice::Unset)
    test(mo7.iid[4] == 3)
    test(mo7.sid == Ice::Unset)
    test(mo7.fs == mo1.fs)
    test(mo7.vs == Ice::Unset)

    test(mo7.shs == mo1.shs)
    test(mo7.es == Ice::Unset)
    test(mo7.fss[0] == Test::FixedStruct.new(78))
    test(mo7.vss == Ice::Unset)
    test(mo7.oos[0].a == 15)
    test(mo7.oops == Ice::Unset)

    test(mo7.ied == Ice::Unset)
    test(mo7.ifsd[4] == Test::FixedStruct.new(78))
    test(mo7.ivsd == Ice::Unset)
    test(mo7.iood[5].a == 15)
    test(mo7.ioopd == Ice::Unset)

    test(mo7.bos == [false, true, false])

    # Clear the second half of the optional members
    mo8 = Test::MultiOptional.new
    mo8.a = mo5.a
    mo8.c = mo5.c
    mo8.e = mo5.e
    mo8.g = mo5.g
    mo8.i = mo5.i
    mo8.k = mo8
    mo8.ss = mo5.ss
    mo8.sid = mo5.sid
    mo8.vs = mo5.vs

    mo8.es = mo5.es
    mo8.vss = mo5.vss
    mo8.oops = mo5.oops

    mo8.ied = mo5.ied
    mo8.ivsd = mo5.ivsd
    mo8.ioopd = mo5.ioopd

    mo9 = initial.pingPong(mo8)
    test(mo9.a == mo1.a)
    test(mo9.b == Ice::Unset)
    test(mo9.c == mo1.c)
    test(mo9.d == Ice::Unset)
    test(mo9.e == mo1.e)
    test(mo9.f == Ice::Unset)
    test(mo9.g == mo1.g)
    test(mo9.h == Ice::Unset)
    test(mo9.i == mo1.i)
    test(mo9.j == Ice::Unset)
    test(mo9.k == mo9)
    test(mo9.bs == Ice::Unset)
    test(mo9.ss == mo1.ss)
    test(mo9.iid == Ice::Unset)
    test(mo9.sid["test"] == 10)
    test(mo9.fs == Ice::Unset)
    test(mo9.vs == mo1.vs)

    test(mo9.shs == Ice::Unset)
    test(mo9.es[0] == Test::MyEnum::MyEnumMember && mo1.es[1] == Test::MyEnum::MyEnumMember)
    test(mo9.fss == Ice::Unset)
    test(mo9.vss[0] == Test::VarStruct.new("hello"))
    test(mo9.oos == Ice::Unset)
    test(mo9.oops[0] == communicator.stringToProxy("test"))

    test(mo9.ied[4] == Test::MyEnum::MyEnumMember)
    test(mo9.ifsd == Ice::Unset)
    test(mo9.ivsd[5] == Test::VarStruct.new("hello"))
    test(mo9.iood == Ice::Unset)
    test(mo9.ioopd[5] == communicator.stringToProxy("test"))

    test(mo9.bos == Ice::Unset)

    #
    # Use the 1.0 encoding with operations whose only class parameters are optional.
    #
    initial.sendOptionalClass(true, Test::OneOptional.new(53))
    initial.ice_encodingVersion(Ice::Encoding_1_0).sendOptionalClass(true, Test::OneOptional.new(53))

    r = initial.returnOptionalClass(true)
    test(r != Ice::Unset)
    r = initial.ice_encodingVersion(Ice::Encoding_1_0).returnOptionalClass(true)
    test(r == Ice::Unset)

    recursive1 = [ Test::Recursive.new ]
    recursive2 = [ Test::Recursive.new ]
    recursive1[0].value = recursive2;

    outer = Test::Recursive.new
    outer.value = recursive1
    initial.pingPong(outer)

    g = Test::G.new
    g.gg1Opt = Test::G1.new("gg1Opt")
    g.gg2 = Test::G2.new(10)
    g.gg2Opt = Test::G2.new(20)
    g.gg1 = Test::G1.new("gg1")
    r = initial.opG(g)
    test(r.gg1Opt.a == "gg1Opt")
    test(r.gg2.a == 10)
    test(r.gg2Opt.a == 20)
    test(r.gg1.a == "gg1")

    initial2 = Test::Initial2Prx::uncheckedCast(base)
    initial2.opVoid(15, "test")

    puts "ok"

    print "testing marshaling of large containers with fixed size elements... "
    STDOUT.flush

    mc = Test::MultiOptional.new

    mc.bs = []
    for i in (0...1000)
        mc.bs.push(0)
    end
    mc.shs = []
    for i in (0...300)
        mc.shs.push(0)
    end

    mc.fss = []
    for i in (0...300)
        mc.fss.push(Test::FixedStruct.new)
    end

    mc.ifsd = {}
    for i in (0...300)
        mc.ifsd[i] = Test::FixedStruct.new
    end

    mc = initial.pingPong(mc)
    test(mc.bs.length == 1000)
    test(mc.shs.length == 300)
    test(mc.fss.length == 300)
    test(mc.ifsd.length == 300)

    puts "ok"

    print "testing tag marshaling... "
    STDOUT.flush

    b = Test::B.new
    b2 = initial.pingPong(b)
    test(b2.ma == Ice::Unset)
    test(b2.mb == Ice::Unset)
    test(b2.mc == Ice::Unset)

    b.ma = 10
    b.mb = 11
    b.mc = 12
    b.md = 13

    b2 = initial.pingPong(b)
    test(b2.ma == 10)
    test(b2.mb == 11)
    test(b2.mc == 12)
    test(b2.md == 13)

    puts "ok"

    print "testing marshalling of objects with optional objects..."
    STDOUT.flush

    f = Test::F.new

    f.af = Test::A.new
    f.ae = f.af

    rf = initial.pingPong(f)
    test(rf.ae == rf.af)

    puts "ok"

    print "testing optional with default values... "
    STDOUT.flush

    wd = initial.pingPong(Test::WD.new)
    test(wd.a == 5)
    test(wd.s == "test")
    wd.a = Ice::Unset
    wd.s = Ice::Unset
    wd = initial.pingPong(wd)
    test(wd.a == Ice::Unset)
    test(wd.s == Ice::Unset)

    puts "ok"

    if communicator.getProperties().getPropertyAsInt("Ice.Default.SlicedFormat") > 0
        print "testing marshaling with unknown class slices... "
        STDOUT.flush

        c = Test::C.new
        c.ss = "test"
        c.ms = "testms"
        c = initial.pingPong(c)
        test(c.ma == Ice::Unset)
        test(c.mb == Ice::Unset)
        test(c.mc == Ice::Unset)
        test(c.md == Ice::Unset)
        test(c.ss == "test")
        test(c.ms == "testms")

        puts "ok"

        print "testing optionals with unknown classes... "
        STDOUT.flush

        initial2 = Test::Initial2Prx::uncheckedCast(base)
        d = Test::D.new
        d.ds = "test"
        d.seq = ["test1", "test2", "test3", "test4"]
        d.ao = Test::A.new(18)
        d.requiredB = 14;
        d.requiredA = 14;
        initial2.opClassAndUnknownOptional(Test::A.new, d)

        puts "ok"
    end

    print "testing optional parameters... "
    STDOUT.flush

    p2, p3 = initial.opByte(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opByte(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opBool(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opBool(true)
    test(p2 == true && p3 == true)

    p2, p3 = initial.opShort(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opShort(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opInt(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opInt(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opLong(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opLong(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opFloat(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opFloat(1.0)
    test(p2 == 1.0 && p3 == 1.0)

    p2, p3 = initial.opDouble(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opDouble(1.0)
    test(p2 == 1.0 && p3 == 1.0)

    p2, p3 = initial.opString(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opString("test")
    test(p2 == "test" && p3 == "test")

    p2, p3 = initial.opMyEnum(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p2, p3 = initial.opMyEnum(Test::MyEnum::MyEnumMember)
    test(p2 == Test::MyEnum::MyEnumMember && p3 == Test::MyEnum::MyEnumMember)

    p2, p3 = initial.opSmallStruct(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = Test::SmallStruct.new(56)
    p2, p3 = initial.opSmallStruct(p1)
    test(p2 == p1 && p3 == p1)
    p2, p3 = initial.opSmallStruct(nil) # Test null struct
    test(p2.m == 0 && p3.m == 0)

    p2, p3 = initial.opFixedStruct(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = Test::FixedStruct.new(56)
    p2, p3 = initial.opFixedStruct(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opVarStruct(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = Test::VarStruct.new("test")
    p2, p3 = initial.opVarStruct(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opOneOptional(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    if initial.supportsNullOptional() then
        p2, p3 = initial.opOneOptional(nil)
        test(p2 == nil && p3 == nil)
    end
    p1 = Test::OneOptional.new(58)
    p2, p3 = initial.opOneOptional(p1)
    test(p2.a == p1.a && p3.a == p1.a)

    p2, p3 = initial.opOneOptionalProxy(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = communicator.stringToProxy("test")
    p2, p3 = initial.opOneOptionalProxy(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opByteSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opByteSeq(p1)
    test(p2.length == p1.length && p3.length == p1.length)
    test(p2[0] == "\x38" || p2[0] == 0x38)
    test(p3[0] == "\x38" || p3[0] == 0x38)

    p2, p3 = initial.opBoolSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(true)
    end
    p2, p3 = initial.opBoolSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opShortSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opShortSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opIntSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opIntSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opLongSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opLongSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opFloatSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(1.0)
    end
    p2, p3 = initial.opFloatSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opDoubleSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push(1.0)
    end
    p2, p3 = initial.opDoubleSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opStringSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...100)
        p1.push("test1")
    end
    p2, p3 = initial.opStringSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opSmallStructSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...10)
        p1.push(Test::SmallStruct.new(1))
    end
    p2, p3 = initial.opSmallStructSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opSmallStructList(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...10)
        p1.push(Test::SmallStruct.new(1))
    end
    p2, p3 = initial.opSmallStructList(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opFixedStructSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...10)
        p1.push(Test::FixedStruct.new(1))
    end
    p2, p3 = initial.opFixedStructSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opFixedStructList(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...10)
        p1.push(Test::FixedStruct.new(1))
    end
    p2, p3 = initial.opFixedStructList(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opVarStructSeq(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = []
    for x in (0...10)
        p1.push(Test::VarStruct.new("test"))
    end
    p2, p3 = initial.opVarStructSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opIntIntDict(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = {1=>2, 2=>3}
    p2, p3 = initial.opIntIntDict(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opStringIntDict(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = {"1"=>2, "2"=>3}
    p2, p3 = initial.opStringIntDict(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opIntOneOptionalDict(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)
    p1 = {1=> Test::OneOptional.new(58), 2=>Test::OneOptional.new(59)}
    p2, p3 = initial.opIntOneOptionalDict(p1)
    test(p2[1].a == 58 && p3[1].a == 58)

    puts "ok"

    print "testing exception optionals... "
    STDOUT.flush

    begin
        initial.opOptionalException(Ice::Unset, Ice::Unset, Ice::Unset)
    rescue Test::OptionalException => ex
        test(ex.a == Ice::Unset)
        test(ex.b == Ice::Unset)
        test(ex.o == Ice::Unset)
    end

    begin
        initial.opOptionalException(30, "test", Test::OneOptional.new(53))
    rescue Test::OptionalException => ex
        test(ex.a == 30)
        test(ex.b == "test")
        test(ex.o.a == 53)
    end

    begin
        #
        # Use the 1.0 encoding with an exception whose only class members are optional.
        #
        initial.ice_encodingVersion(Ice::Encoding_1_0).opOptionalException(30, "test", Test::OneOptional.new(53))
    rescue Test::OptionalException => ex
        test(ex.a == Ice::Unset)
        test(ex.b == Ice::Unset)
        test(ex.o == Ice::Unset)
    end

    begin
        initial.opDerivedException(Ice::Unset, Ice::Unset, Ice::Unset)
    rescue Test::DerivedException => ex
        test(ex.a == Ice::Unset)
        test(ex.b == Ice::Unset)
        test(ex.o == Ice::Unset)
        test(ex.ss == Ice::Unset)
        test(ex.o2 == Ice::Unset)
    end

    begin
        initial.opDerivedException(30, "test", Test::OneOptional.new(53))
    rescue Test::DerivedException => ex
        test(ex.a == 30)
        test(ex.b == "test")
        test(ex.o.a == 53)
        test(ex.ss == "test")
        test(ex.o2 == ex.o)
    end

    begin
        initial.opRequiredException(Ice::Unset, Ice::Unset, Ice::Unset)
    rescue Test::RequiredException => ex
        test(ex.a == Ice::Unset)
        test(ex.b == Ice::Unset)
        test(ex.o == Ice::Unset)
        test(ex.ss != Ice::Unset)
        test(ex.o2 != Ice::Unset)
    end

    begin
        initial.opRequiredException(30, "test", Test::OneOptional.new(53))
    rescue Test::RequiredException => ex
        test(ex.a == 30)
        test(ex.b == "test")
        test(ex.o.a == 53)
        test(ex.ss == "test")
        test(ex.o2 == ex.o)
    end

    puts "ok"

    print "testing optionals with marshaled results... "
    STDOUT.flush

    # TODO: Fix bug ICE-7276
    #test(initial.opMStruct1() != Ice::Unset)
    test(initial.opMDict1() != Ice::Unset)
    test(initial.opMSeq1() != Ice::Unset)
    test(initial.opMG1() != Ice::Unset)

    (p3, p2) = initial.opMStruct2(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)

    p1 = Test::SmallStruct.new()
    (p3, p2) = initial.opMStruct2(p1)
    test(p2 == p1 && p3 == p1)

    (p3, p2) = initial.opMSeq2(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)

    p1 = ["hello"]
    (p3, p2) = initial.opMSeq2(p1)
    test(p2[0] == "hello" && p3[0] == "hello")

    (p3, p2) = initial.opMDict2(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)

    p1 = {"test" => 54}
    (p3, p2) = initial.opMDict2(p1)
    test(p2["test"] == 54 && p3["test"] == 54)

    (p3, p2) = initial.opMG2(Ice::Unset)
    test(p2 == Ice::Unset && p3 == Ice::Unset)

    p1 = Test::G.new()
    (p3, p2) = initial.opMG2(p1)
    test(p2 != Ice::Unset && p3 != Ice::Unset && p3 == p2)

    puts "ok"

    return initial
end
