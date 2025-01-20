# Copyright (c) ZeroC, Inc.

def allTests(helper, communicator)
    ref = "initial:#{helper.getTestEndpoint()}"
    initial = Test::InitialPrx.new(communicator, ref)

    print "testing optional data members... "
    STDOUT.flush

    oo1 = Test::OneOptional.new
    test(oo1.a == nil)
    oo1.a = 15

    oo2 = Test::OneOptional.new(16)
    test(oo2.a == 16)

    mo1 = Test::MultiOptional.new()
    test(mo1.a == nil)
    test(mo1.b == nil)
    test(mo1.c == nil)
    test(mo1.d == nil)
    test(mo1.e == nil)
    test(mo1.f == nil)
    test(mo1.g == nil)
    test(mo1.h == nil)
    test(mo1.i == nil)
    test(mo1.j == nil)
    test(mo1.bs == nil)
    test(mo1.ss == nil)
    test(mo1.iid == nil)
    test(mo1.sid == nil)
    test(mo1.fs == nil)
    test(mo1.vs == nil)

    test(mo1.shs == nil)
    test(mo1.es == nil)
    test(mo1.fss == nil)
    test(mo1.vss == nil)
    test(mo1.mips == nil)

    test(mo1.ied == nil)
    test(mo1.ifsd == nil)
    test(mo1.ivsd == nil)
    test(mo1.imipd == nil)

    test(mo1.bos == nil)

    ss = Test::SmallStruct.new()
    fs = Test::FixedStruct.new(78)
    vs = Test::VarStruct.new("hello")
    mo1 = Test::MultiOptional.new(15, true, 19, 78, 99, 5.5, 1.0, "test", Test::MyEnum::MyEnumMember, \
                                  Test::MyInterfacePrx.new(communicator, "test"), \
                                  [5], ["test", "test2"], {4=>3}, {"test"=>10}, fs, vs, [1], \
                                  [Test::MyEnum::MyEnumMember, Test::MyEnum::MyEnumMember], [ fs ], [ vs ], \
                                  [ Test::MyInterfacePrx.new(communicator, "test") ], \
                                  {4=> Test::MyEnum::MyEnumMember}, {4=>fs}, {5=>vs}, \
                                  {5=> Test::MyInterfacePrx.new(communicator, "test")}, \
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
    test(mo1.mips[0] == communicator.stringToProxy("test"))

    test(mo1.ied[4] == Test::MyEnum::MyEnumMember)
    test(mo1.ifsd[4] == Test::FixedStruct.new(78))
    test(mo1.ivsd[5] == Test::VarStruct.new("hello"))
    test(mo1.imipd[5] == communicator.stringToProxy("test"))

    test(mo1.bos == [false, true, false])

    #
    # Test generated struct and classes compare with nil
    #
    test(ss != nil)
    test(fs != nil)
    test(vs != nil)
    test(mo1 != nil)

    puts "ok"

    print "testing marshaling... "
    STDOUT.flush

    oo4 = initial.pingPong(Test::OneOptional.new)
    test(oo4.a == nil)

    oo5 = initial.pingPong(oo1)
    test(oo1.a == oo5.a)

    mo4 = initial.pingPong(Test::MultiOptional.new)
    test(mo4.a == nil)
    test(mo4.b == nil)
    test(mo4.c == nil)
    test(mo4.d == nil)
    test(mo4.e == nil)
    test(mo4.f == nil)
    test(mo4.g == nil)
    test(mo4.h == nil)
    test(mo4.i == nil)
    test(mo4.j == nil)
    test(mo4.bs == nil)
    test(mo4.ss == nil)
    test(mo4.iid == nil)
    test(mo4.sid == nil)
    test(mo4.fs == nil)
    test(mo4.vs == nil)

    test(mo4.shs == nil)
    test(mo4.es == nil)
    test(mo4.fss == nil)
    test(mo4.vss == nil)
    test(mo4.mips == nil)

    test(mo4.ied == nil)
    test(mo4.ifsd == nil)
    test(mo4.ivsd == nil)
    test(mo4.imipd == nil)

    test(mo4.bos == nil)

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
    test(mo5.mips[0] == communicator.stringToProxy("test"))

    test(mo5.ied[4] == Test::MyEnum::MyEnumMember)
    test(mo5.ifsd[4] == Test::FixedStruct.new(78))
    test(mo5.ivsd[5] == Test::VarStruct.new("hello"))
    test(mo5.imipd[5] == communicator.stringToProxy("test"))

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
    mo6.ifsd = mo5.ifsd
    mo6.bos = mo5.bos

    mo7 = initial.pingPong(mo6)
    test(mo7.a == nil)
    test(mo7.b == mo1.b)
    test(mo7.c == nil)
    test(mo7.d == mo1.d)
    test(mo7.e == nil)
    test(mo7.f == mo1.f)
    test(mo7.g == nil)
    test(mo7.h == mo1.h)
    test(mo7.i == nil)
    test(mo7.j == mo1.j)
    test(mo7.bs.unpack("C*") == [0x05])
    test(mo7.ss == nil)
    test(mo7.iid[4] == 3)
    test(mo7.sid == nil)
    test(mo7.fs == mo1.fs)
    test(mo7.vs == nil)

    test(mo7.shs == mo1.shs)
    test(mo7.es == nil)
    test(mo7.fss[0] == Test::FixedStruct.new(78))
    test(mo7.vss == nil)
    test(mo7.mips == nil)

    test(mo7.ied == nil)
    test(mo7.ifsd[4] == Test::FixedStruct.new(78))
    test(mo7.ivsd == nil)
    test(mo7.imipd == nil)

    test(mo7.bos == [false, true, false])

    # Clear the second half of the optional members
    mo8 = Test::MultiOptional.new
    mo8.a = mo5.a
    mo8.c = mo5.c
    mo8.e = mo5.e
    mo8.g = mo5.g
    mo8.i = mo5.i
    mo8.ss = mo5.ss
    mo8.sid = mo5.sid
    mo8.vs = mo5.vs

    mo8.es = mo5.es
    mo8.vss = mo5.vss
    mo8.mips = mo5.mips

    mo8.ied = mo5.ied
    mo8.ivsd = mo5.ivsd
    mo8.imipd = mo5.imipd

    mo9 = initial.pingPong(mo8)
    test(mo9.a == mo1.a)
    test(mo9.b == nil)
    test(mo9.c == mo1.c)
    test(mo9.d == nil)
    test(mo9.e == mo1.e)
    test(mo9.f == nil)
    test(mo9.g == mo1.g)
    test(mo9.h == nil)
    test(mo9.i == mo1.i)
    test(mo9.j == nil)
    test(mo9.bs == nil)
    test(mo9.ss == mo1.ss)
    test(mo9.iid == nil)
    test(mo9.sid["test"] == 10)
    test(mo9.fs == nil)
    test(mo9.vs == mo1.vs)

    test(mo9.shs == nil)
    test(mo9.es[0] == Test::MyEnum::MyEnumMember && mo1.es[1] == Test::MyEnum::MyEnumMember)
    test(mo9.fss == nil)
    test(mo9.vss[0] == Test::VarStruct.new("hello"))
    test(mo9.mips[0] == communicator.stringToProxy("test"))

    test(mo9.ied[4] == Test::MyEnum::MyEnumMember)
    test(mo9.ifsd == nil)
    test(mo9.ivsd[5] == Test::VarStruct.new("hello"))
    test(mo9.imipd[5] == communicator.stringToProxy("test"))

    test(mo9.bos == nil)

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

    initial2 = Test::Initial2Prx::uncheckedCast(initial)
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
    test(b2.ma == nil)
    test(b2.mb == nil)
    test(b2.mc == nil)

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

    print "testing marshaling of objects with optional members..."
    STDOUT.flush

    f = Test::F.new

    f.fsf = Test::FixedStruct.new
    f.fse = f.fsf

    rf = initial.pingPong(f)
    test(rf.fse == rf.fsf)

    puts "ok"

    print "testing optional with default values... "
    STDOUT.flush

    wd = initial.pingPong(Test::WD.new)
    test(wd.a == 5)
    test(wd.s == "test")
    wd.a = nil
    wd.s = nil
    wd = initial.pingPong(wd)
    test(wd.a == nil)
    test(wd.s == nil)

    puts "ok"

    if communicator.getProperties().getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0
        print "testing marshaling with unknown class slices... "
        STDOUT.flush

        c = Test::C.new
        c.ss = "test"
        c.ms = "testms"
        c = initial.pingPong(c)
        test(c.ma == nil)
        test(c.mb == nil)
        test(c.mc == nil)
        test(c.md == nil)
        test(c.ss == "test")
        test(c.ms == "testms")

        puts "ok"

        print "testing operations with unknown optionals... "
        STDOUT.flush

        initial2 = Test::Initial2Prx::uncheckedCast(initial)
        ovs = Test::VarStruct.new("test")
        initial2.opClassAndUnknownOptional(Test::A.new, ovs)

        puts "ok"
    end

    print "testing optional parameters... "
    STDOUT.flush

    p2, p3 = initial.opByte(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opByte(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opBool(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opBool(true)
    test(p2 == true && p3 == true)

    p2, p3 = initial.opShort(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opShort(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opInt(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opInt(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opLong(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opLong(56)
    test(p2 == 56 && p3 == 56)

    p2, p3 = initial.opFloat(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opFloat(1.0)
    test(p2 == 1.0 && p3 == 1.0)

    p2, p3 = initial.opDouble(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opDouble(1.0)
    test(p2 == 1.0 && p3 == 1.0)

    p2, p3 = initial.opString(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opString("test")
    test(p2 == "test" && p3 == "test")

    p2, p3 = initial.opMyEnum(nil)
    test(p2 == nil && p3 == nil)
    p2, p3 = initial.opMyEnum(Test::MyEnum::MyEnumMember)
    test(p2 == Test::MyEnum::MyEnumMember && p3 == Test::MyEnum::MyEnumMember)

    p2, p3 = initial.opSmallStruct(nil)
    test(p2 == nil && p3 == nil)
    p1 = Test::SmallStruct.new(56)
    p2, p3 = initial.opSmallStruct(p1)
    test(p2 == p1 && p3 == p1)
    test(Ice::Unset == nil)
    p2, p3 = initial.opSmallStruct(Ice::Unset) # Test backwards compat syntax
    test(p2 == Ice::Unset && p3 == Ice::Unset)

    p2, p3 = initial.opFixedStruct(nil)
    test(p2 == nil && p3 == nil)
    p1 = Test::FixedStruct.new(56)
    p2, p3 = initial.opFixedStruct(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opVarStruct(nil)
    test(p2 == nil && p3 == nil)
    p1 = Test::VarStruct.new("test")
    p2, p3 = initial.opVarStruct(p1)
    test(p2 == p1 && p3 == p1)

    p1 = Test::OneOptional.new()
    p2, p3 = initial.opOneOptional(p1)
    test(p2.a == nil && p3.a == nil)
    p1 = Test::OneOptional.new(58)
    p2, p3 = initial.opOneOptional(p1)
    test(p2.a == p1.a && p3.a == p1.a)

    p2, p3 = initial.opMyInterfaceProxy(nil)
    test(p2 == nil && p3 == nil)
    p1 = Test::MyInterfacePrx.new(communicator, "test")
    p2, p3 = initial.opMyInterfaceProxy(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opByteSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opByteSeq(p1)
    test(p2.length == p1.length && p3.length == p1.length)
    test(p2[0] == "\x38" || p2[0] == 0x38)
    test(p3[0] == "\x38" || p3[0] == 0x38)

    p2, p3 = initial.opBoolSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(true)
    end
    p2, p3 = initial.opBoolSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opShortSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opShortSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opIntSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opIntSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opLongSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(56)
    end
    p2, p3 = initial.opLongSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opFloatSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(1.0)
    end
    p2, p3 = initial.opFloatSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opDoubleSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push(1.0)
    end
    p2, p3 = initial.opDoubleSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opStringSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...100)
        p1.push("test1")
    end
    p2, p3 = initial.opStringSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opSmallStructSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...10)
        p1.push(Test::SmallStruct.new(1))
    end
    p2, p3 = initial.opSmallStructSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opSmallStructList(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...10)
        p1.push(Test::SmallStruct.new(1))
    end
    p2, p3 = initial.opSmallStructList(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opFixedStructSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...10)
        p1.push(Test::FixedStruct.new(1))
    end
    p2, p3 = initial.opFixedStructSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opFixedStructList(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...10)
        p1.push(Test::FixedStruct.new(1))
    end
    p2, p3 = initial.opFixedStructList(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opVarStructSeq(nil)
    test(p2 == nil && p3 == nil)
    p1 = []
    for x in (0...10)
        p1.push(Test::VarStruct.new("test"))
    end
    p2, p3 = initial.opVarStructSeq(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opIntIntDict(nil)
    test(p2 == nil && p3 == nil)
    p1 = {1=>2, 2=>3}
    p2, p3 = initial.opIntIntDict(p1)
    test(p2 == p1 && p3 == p1)

    p2, p3 = initial.opStringIntDict(nil)
    test(p2 == nil && p3 == nil)
    p1 = {"1"=>2, "2"=>3}
    p2, p3 = initial.opStringIntDict(p1)
    test(p2 == p1 && p3 == p1)

    puts "ok"

    print "testing exception optionals... "
    STDOUT.flush

    begin
        initial.opOptionalException(nil, nil)
    rescue Test::OptionalException => ex
        test(ex.a == nil)
        test(ex.b == nil)
    end

    begin
        initial.opOptionalException(30, "test")
    rescue Test::OptionalException => ex
        test(ex.a == 30)
        test(ex.b == "test")
    end

    begin
        #
        # Use the 1.0 encoding with an exception whose only data members are optional.
        #
        initial.ice_encodingVersion(Ice::Encoding_1_0).opOptionalException(30, "test")
    rescue Test::OptionalException => ex
        test(ex.a == nil)
        test(ex.b == nil)
    end

    begin
        initial.opDerivedException(nil, nil)
    rescue Test::DerivedException => ex
        test(ex.a == nil)
        test(ex.b == nil)
        test(ex.ss == nil)
        test(ex.d1 == "d1")
        test(ex.d2 == "d2")
    end

    begin
        initial.opDerivedException(30, "test")
    rescue Test::DerivedException => ex
        test(ex.a == 30)
        test(ex.b == "test")
        test(ex.ss == "test")
        test(ex.d1 == "d1")
        test(ex.d2 == "d2")
    end

    begin
        initial.opRequiredException(nil, nil)
    rescue Test::RequiredException => ex
        test(ex.a == nil)
        test(ex.b == nil)
        test(ex.ss != nil)
    end

    begin
        initial.opRequiredException(30, "test")
    rescue Test::RequiredException => ex
        test(ex.a == 30)
        test(ex.b == "test")
        test(ex.ss == "test")
    end

    puts "ok"

    print "testing optionals with marshaled results... "
    STDOUT.flush

    test(initial.opMStruct1() != nil)
    test(initial.opMDict1() != nil)
    test(initial.opMSeq1() != nil)

    (p3, p2) = initial.opMStruct2(nil)
    test(p2 == nil && p3 == nil)

    p1 = Test::SmallStruct.new()
    (p3, p2) = initial.opMStruct2(p1)
    test(p2 == p1 && p3 == p1)

    (p3, p2) = initial.opMSeq2(nil)
    test(p2 == nil && p3 == nil)

    p1 = ["hello"]
    (p3, p2) = initial.opMSeq2(p1)
    test(p2[0] == "hello" && p3[0] == "hello")

    (p3, p2) = initial.opMDict2(nil)
    test(p2 == nil && p3 == nil)

    p1 = {"test" => 54}
    (p3, p2) = initial.opMDict2(p1)
    test(p2["test"] == 54 && p3["test"] == 54)

    puts "ok"

    return initial
end
