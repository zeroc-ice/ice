# Copyright (c) ZeroC, Inc.

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(helper, communicator)
    initial = Test::InitialPrx.new(communicator, "initial:#{helper.getTestEndpoint()}")

    print "getting B1... "
    STDOUT.flush
    b1 = initial.getB1()
    test(b1)
    puts "ok"

    print "getting B2... "
    STDOUT.flush
    b2 = initial.getB2()
    test(b2)
    puts "ok"

    print "getting C... "
    STDOUT.flush
    c = initial.getC()
    test(c)
    puts "ok"

    print "getting D... "
    STDOUT.flush
    d = initial.getD()
    test(d)
    puts "ok"

    print "checking consistency... "
    STDOUT.flush
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theB == b1)
    test(b1.theC == nil)
    test(b1.theA.is_a?(Test::B))
    test(b1.theA.theA == b1.theA)
    test(b1.theA.theB == b1)
    test(b1.theA.theC)
    test(b1.theA.theC.theB == b1.theA)
    test(b1.preMarshalInvoked)
    test(b1.postUnmarshalInvoked)
    test(b1.theA.preMarshalInvoked)
    test(b1.theA.postUnmarshalInvoked)
    test(b1.theA.theC.preMarshalInvoked)
    test(b1.theA.theC.postUnmarshalInvoked)
    # More tests possible for b2 and d, but I think this is already sufficient.
    test(b2.theA == b2)
    test(d.theC == nil)
    puts "ok"

    print "getting B1, B2, C, and D all at once... "
    STDOUT.flush
    b1, b2, c, d = initial.getAll()
    test(b1)
    test(b2)
    test(c)
    test(d)
    puts "ok"

    print "checking consistency... "
    STDOUT.flush
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theA == b2)
    test(b1.theB == b1)
    test(b1.theC == nil)
    test(b2.theA == b2)
    test(b2.theB == b1)
    test(b2.theC == c)
    test(c.theB == b2)
    test(d.theA == b1)
    test(d.theB == b2)
    test(d.theC == nil)
    test(d.preMarshalInvoked)
    test(d.postUnmarshalInvoked)
    test(d.theA.preMarshalInvoked)
    test(d.theA.postUnmarshalInvoked)
    test(d.theB.preMarshalInvoked)
    test(d.theB.postUnmarshalInvoked)
    test(d.theB.theC.preMarshalInvoked)
    test(d.theB.theC.postUnmarshalInvoked)
    puts "ok"

    print "getting K... "
    STDOUT.flush
    k = initial.getK()
    test(k.value.data == "l")
    puts "ok"

    print "testing Value as parameter... "
    STDOUT.flush
    v1 = Test::L.new("l")
    v2, v3 = initial.opValue(v1)
    test(v2.data == "l")
    test(v3.data == "l")

    v1 = [Test::L.new("l")]
    v2, v3 = initial.opValueSeq(v1)
    test(v2[0].data == "l")
    test(v3[0].data == "l")

    v1 = {}
    v1["l"] = Test::L.new("l")
    v2, v3 = initial.opValueMap(v1)
    test(v2["l"].data == "l")
    test(v3["l"].data == "l")
    puts "ok"

    print "getting D1... "
    STDOUT.flush
    d1 = initial.getD1(Test::D1.new(Test::A1.new("a1"), Test::A1.new("a2"), Test::A1.new("a3"), Test::A1.new("a4")))
    test(d1.a1.name == "a1")
    test(d1.a2.name == "a2")
    test(d1.a3.name == "a3")
    test(d1.a4.name == "a4")
    puts "ok"

    print "throw EDerived... "
    STDOUT.flush
    begin
        initial.throwEDerived()
        test(false)
    rescue Test::EDerived => e
        test(e.a1.name == "a1")
        test(e.a2.name == "a2")
        test(e.a3.name == "a3")
        test(e.a4.name == "a4")
    end
    puts "ok"

    print "setting G... "
    STDOUT.flush
    begin
        initial.setG(Test::G.new(Test::S.new("hello"), "g"))
    rescue Ice::OperationNotExistException
    end
    puts "ok"

    print "testing sequences... "
    STDOUT.flush
    initial.opBaseSeq([])

    retS, outS = initial.opBaseSeq([Test::Base.new])
    test(retS.length == 1 && outS.length == 1)
    puts "ok"

    print "testing recursive type... "
    STDOUT.flush
    top = Test::Recursive.new
    bottom = top;
    maxDepth = 9;

    maxDepth.times do |_|
        bottom.v = Test::Recursive.new
        bottom = bottom.v;
    end
    initial.setRecursive(top)

    # Adding one more level would exceed the max class graph depth
    bottom.v = Test::Recursive.new
    bottom = bottom.v;

    begin
        initial.setRecursive(top)
        test(false)
    rescue Ice::UnknownLocalException
        # Expected marshal exception from the server (max class graph depth reached)
    end
    initial.setRecursive(Test::Recursive.new)
    puts "ok"

    print "testing compact ID... "
    STDOUT.flush
    begin
        r = initial.getCompact()
        test(r != nil)
    rescue Ice::OperationNotExistException
    end
    puts "ok"

    print "testing marshaled results..."
    STDOUT.flush
    b1 = initial.getMB()
    test(b1 != nil && b1.theB == b1);
    b1 = initial.getAMDMB()
    test(b1 != nil && b1.theB == b1);
    puts "ok"

    print "testing UnexpectedObjectException... "
    STDOUT.flush
    ref = "uoet:#{helper.getTestEndpoint()}"
    uoet = Test::UnexpectedObjectExceptionTestPrx.new(communicator, ref)
    begin
        uoet.op()
        test(false)
    rescue Ice::MarshalException => ex
        test(ex.message["::Test::AlsoEmpty"])
        test(ex.message["::Test::Empty"])
    rescue Ice::Exception => ex
        puts $!
        print ex.backtrace.join("\n")
        test(false)
    rescue => ex
        puts $!
        print ex.backtrace.join("\n")
        test(false)
    end
    puts "ok"

    print "testing class containing complex dictionary... "
    STDOUT.flush
    m = Test::M.new
    m.v = {}
    k1 = Test::StructKey.new(1, "1")
    m.v[k1] = Test::L.new("one")
    k2 = Test::StructKey.new(2, "2")
    m.v[k2] = Test::L.new("two")
    m1, m2 = initial.opM(m)
    test(m1.v.length == 2)
    test(m2.v.length == 2)

    test(m1.v[k1].data == "one")
    test(m2.v[k1].data == "one")

    test(m1.v[k2].data == "two")
    test(m2.v[k2].data == "two")
    puts "ok"

    print "testing forward declarations... "
    STDOUT.flush
    f11, f12 = initial.opF1(Test::F1.new("F11"))
    test(f11.name == "F11")
    test(f12.name == "F12")

    f21, f22 = initial.opF2(Test::F2Prx.new(communicator, "F21:#{helper.getTestEndpoint()}"))
    test(f21.ice_getIdentity().name == "F21")
    f21.op()
    test(f22.ice_getIdentity().name == "F22")

    if initial.hasF3() then
        f31, f32 = initial.opF3(Test::F3.new(f11, f21))
        test(f31.f1.name == "F11")
        test(f31.f2.ice_getIdentity().name == "F21")

        test(f32.f1.name == "F12")
        test(f32.f2.ice_getIdentity().name == "F22")
    end
    puts "ok"

    print "testing sending class cycle... "
    STDOUT.flush
    rec = Test::Recursive.new
    rec.v = rec
    acceptsCycles = initial.acceptsClassCycles()
    begin
        initial.setCycle(rec)
        test(acceptsCycles)
    rescue Ice::UnknownLocalException => ex
        test(!acceptsCycles)
    end
    puts "ok"

    return initial
end
