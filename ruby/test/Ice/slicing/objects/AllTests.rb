#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require 'Ice'
Ice::loadSlice('-I. --all ClientPrivate.ice')

def allTests(helper, communicator)
    obj = communicator.stringToProxy("Test:#{helper.getTestEndpoint()}")
    t = Test::TestIntfPrx::checkedCast(obj)

    print "base as Object... "
    STDOUT.flush
    o = nil
    begin
        o = t.SBaseAsObject()
        test(o)
        test(o.ice_id() == "::Test::SBase")
    rescue Ice::Exception
        puts $!.inspect
        test(false)
    end
    sb = o
    test(sb.is_a?(Test::SBase))
    test(sb)
    test(sb.sb == "SBase.sb")
    puts "ok"

    print "base as base... "
    STDOUT.flush
    begin
        sb = t.SBaseAsSBase()
        test(sb.sb == "SBase.sb")
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "base with known derived as base... "
    STDOUT.flush
    begin
        sb = t.SBSKnownDerivedAsSBase()
        test(sb.sb == "SBSKnownDerived.sb")
    rescue Ice::Exception
        test(false)
    end
    sbskd = sb
    test(sbskd.is_a?(Test::SBSKnownDerived))
    test(sbskd)
    test(sbskd.sbskd == "SBSKnownDerived.sbskd")
    puts "ok"

    print "base with known derived as known derived... "
    STDOUT.flush
    begin
        sbskd = t.SBSKnownDerivedAsSBSKnownDerived()
        test(sbskd.sbskd == "SBSKnownDerived.sbskd")
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "base with unknown derived as base... "
    STDOUT.flush
    begin
        sb = t.SBSUnknownDerivedAsSBase()
        test(sb.sb == "SBSUnknownDerived.sb")
    rescue Ice::Exception
        test(false)
    end
    if t.ice_getEncodingVersion() == Ice::Encoding_1_0
        begin
            #
            # This test succeeds for the 1.0 encoding.
            #
            sb = t.SBSUnknownDerivedAsSBaseCompact()
            test(sb.sb == "SBSUnknownDerived.sb")
        rescue
            test(false)
        end
    else
        begin
            #
            # This test fails when using the compact format because the instance cannot
            # be sliced to a known type.
            #
            sb = t.SBSUnknownDerivedAsSBaseCompact()
            test(false)
        rescue Ice::NoValueFactoryException
            # Expected.
        rescue
            test(false)
        end
    end
    puts "ok"

    print "unknown with Object as Object... "
    STDOUT.flush
    begin
        o = t.SUnknownAsObject()
        test(t.ice_getEncodingVersion() != Ice::Encoding_1_0)
        test(o.is_a?(Ice::UnknownSlicedValue))
        test(o.ice_id() == "::Test::SUnknown")
        test(o.ice_getSlicedData() != nil)
        t.checkSUnknown(o)
    rescue Ice::NoValueFactoryException
        test(t.ice_getEncodingVersion() == Ice::Encoding_1_0)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "one-element cycle... "
    STDOUT.flush
    begin
        b = t.oneElementCycle()
        test(b)
        test(b.ice_id() == "::Test::B")
        test(b.sb == "B1.sb")
        test(b.pb == b)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "two-element cycle... "
    STDOUT.flush
    begin
        b1 = t.twoElementCycle()
        test(b1)
        test(b1.ice_id() == "::Test::B")
        test(b1.sb == "B1.sb")

        b2 = b1.pb
        test(b2)
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "B2.sb")
        test(b2.pb == b1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "known derived pointer slicing as base... "
    STDOUT.flush
    begin
        b1 = t.D1AsB()
        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb)
        test(b1.pb != b1)
        d1 = b1
        test(d1.is_a?(Test::D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1)
        test(d1.pd1 != b1)
        test(b1.pb == d1.pd1)

        b2 = b1.pb
        test(b2)
        test(b2.pb == b1)
        test(b2.sb == "D2.sb")
        test(b2.ice_id() == "::Test::B")
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "known derived pointer slicing as derived... "
    STDOUT.flush
    begin
        d1 = t.D1AsD1()
        test(d1)
        test(d1.ice_id() == "::Test::D1")
        test(d1.sb == "D1.sb")
        test(d1.pb)
        test(d1.pb != d1)

        b2 = d1.pb
        test(b2)
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "D2.sb")
        test(b2.pb == d1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "unknown derived pointer slicing as base... "
    STDOUT.flush
    begin
        b2 = t.D2AsB()
        test(b2)
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "D2.sb")
        test(b2.pb)
        test(b2.pb != b2)

        b1 = b2.pb
        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        test(d1.is_a?(Test::D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "param ptr slicing with known first... "
    STDOUT.flush
    begin
        b1, b2 = t.paramTest1()

        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        test(d1.is_a?(Test::D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        test(b2)
        test(b2.ice_id() == "::Test::B")        # No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "param ptr slicing with unknown first... "
    STDOUT.flush
    begin
        b2, b1 = t.paramTest2()

        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        test(d1.is_a?(Test::D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        test(b2)
        test(b2.ice_id() == "::Test::B")        # No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "return value identity with known first... "
    STDOUT.flush
    begin
        r, p1, p2 = t.returnTest1()
        test(r == p1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "return value identity with unknown first... "
    STDOUT.flush
    begin
        r, p1, p2 = t.returnTest2()
        test(r == p1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "return value identity for input params known first... "
    STDOUT.flush
    begin
        d1 = Test::D1.new
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = Test::D3.new
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        b1 = t.returnTest3(d1, d3)

        test(b1)
        test(b1.sb == "D1.sb")
        test(b1.ice_id() == "::Test::D1")
        p1 = b1
        test(p1.is_a?(Test::D1))
        test(p1.sd1 == "D1.sd1")
        test(p1.pd1 == b1.pb)

        b2 = b1.pb
        test(b2)
        test(b2.sb == "D3.sb")
        test(b2.ice_id() == "::Test::B")        # Sliced by server
        test(b2.pb == b1)
        p3 = b2
        test(!p3.is_a?(Test::D3))

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "return value identity for input params unknown first... "
    STDOUT.flush
    begin
        d1 = Test::D1.new
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = Test::D3.new
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        b1 = t.returnTest3(d3, d1)

        test(b1)
        test(b1.sb == "D3.sb")
        test(b1.ice_id() == "::Test::B")        # Sliced by server
        p1 = b1
        test(!p1.is_a?(Test::D3))

        b2 = b1.pb
        test(b2)
        test(b2.sb == "D1.sb")
        test(b2.ice_id() == "::Test::D1")
        test(b2.pb == b1)
        p3 = b2
        test(p3.is_a?(Test::D1))
        test(p3.sd1 == "D1.sd1")
        test(p3.pd1 == b1)

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "remainder unmarshaling (3 instances)... "
    STDOUT.flush
    begin
        ret, p1, p2 = t.paramTest3()

        test(p1)
        test(p1.sb == "D2.sb (p1 1)")
        test(p1.pb == nil)
        test(p1.ice_id() == "::Test::B")

        test(p2)
        test(p2.sb == "D2.sb (p2 1)")
        test(p2.pb == nil)
        test(p2.ice_id() == "::Test::B")

        test(ret)
        test(ret.sb == "D1.sb (p2 2)")
        test(ret.pb == nil)
        test(ret.ice_id() == "::Test::D1")
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "remainder unmarshaling (4 instances)... "
    STDOUT.flush
    begin
        ret, b = t.paramTest4()

        test(b)
        test(b.sb == "D4.sb (1)")
        test(b.pb == nil)
        test(b.ice_id() == "::Test::B")

        test(ret)
        test(ret.sb == "B.sb (2)")
        test(ret.pb == nil)
        test(ret.ice_id() == "::Test::B")
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "param ptr slicing, instance marshaled in unknown derived as base... "
    STDOUT.flush
    begin
        b1 = Test::B.new
        b1.sb = "B.sb(1)"
        b1.pb = b1

        d3 = Test::D3.new
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = b1

        b2 = Test::B.new
        b2.sb = "B.sb(2)"
        b2.pb = b1

        r = t.returnTest3(d3, b2)

        test(r)
        test(r.ice_id() == "::Test::B")
        test(r.sb == "D3.sb")
        test(r.pb == r)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "param ptr slicing, instance marshaled in unknown derived as derived... "
    STDOUT.flush
    begin
        d11 = Test::D1.new
        d11.sb = "D1.sb(1)"
        d11.pb = d11
        d11.sd1 = "D1.sd1(1)"
        d11.pd1 = nil

        d3 = Test::D3.new
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = d11

        d12 = Test::D1.new
        d12.sb = "D1.sb(2)"
        d12.pb = d12
        d12.sd1 = "D1.sd1(2)"
        d12.pd1 = d11

        r = t.returnTest3(d3, d12)
        test(r)
        test(r.ice_id() == "::Test::B")
        test(r.sb == "D3.sb")
        test(r.pb == r)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "sequence slicing... "
    STDOUT.flush
    begin
        ss = Test::SS3.new
        ss1b = Test::B.new
        ss1b.sb = "B.sb"
        ss1b.pb = ss1b

        ss1d1 = Test::D1.new
        ss1d1.sb = "D1.sb"
        ss1d1.sd1 = "D1.sd1"
        ss1d1.pb = ss1b

        ss1d3 = Test::D3.new
        ss1d3.sb = "D3.sb"
        ss1d3.sd3 = "D3.sd3"
        ss1d3.pb = ss1b

        ss2b = Test::B.new
        ss2b.sb = "B.sb"
        ss2b.pb = ss1b

        ss2d1 = Test::D1.new
        ss2d1.sb = "D1.sb"
        ss2d1.sd1 = "D1.sd1"
        ss2d1.pb = ss2b

        ss2d3 = Test::D3.new
        ss2d3.sb = "D3.sb"
        ss2d3.sd3 = "D3.sd3"
        ss2d3.pb = ss2b

        ss1d1.pd1 = ss2b
        ss1d3.pd3 = ss2d1

        ss2d1.pd1 = ss1d3
        ss2d3.pd3 = ss1d1

        ss1 = Test::SS1.new
        ss1.s = [ss1b, ss1d1, ss1d3]

        ss2 = Test::SS2.new
        ss2.s = [ss2b, ss2d1, ss2d3]

        ss = t.sequenceTest(ss1, ss2)

        test(ss.c1)
        ss1b = ss.c1.s[0]
        ss1d1 = ss.c1.s[1]
        test(ss.c2)
        ss1d3 = ss.c1.s[2]

        test(ss.c2)
        ss2b = ss.c2.s[0]
        ss2d1 = ss.c2.s[1]
        ss2d3 = ss.c2.s[2]

        test(ss1b.pb == ss1b)
        test(ss1d1.pb == ss1b)
        test(ss1d3.pb == ss1b)

        test(ss2b.pb == ss1b)
        test(ss2d1.pb == ss2b)
        test(ss2d3.pb == ss2b)

        test(ss1b.ice_id() == "::Test::B")
        test(ss1d1.ice_id() == "::Test::D1")
        test(ss1d3.ice_id() == "::Test::B")

        test(ss2b.ice_id() == "::Test::B")
        test(ss2d1.ice_id() == "::Test::D1")
        test(ss2d3.ice_id() == "::Test::B")
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "dictionary slicing... "
    STDOUT.flush
    begin
        bin = {}
        for i in (0...10)
            d1 = Test::D1.new
            s = "D1." + i.to_s
            d1.sb = s
            d1.pb = d1
            d1.sd1 = s
            d1.pd1 = nil
            bin[i] = d1
        end

        r, bout = t.dictionaryTest(bin)

        test(bout.length == 10)
        for i in (0...10)
            b = bout[i * 10]
            test(b)
            s = "D1." + i.to_s
            test(b.sb == s)
            test(b.pb)
            test(b.pb != b)
            test(b.pb.sb == s)
            test(b.pb.pb == b.pb)
        end

        test(r.length == 10)
        for i in (0...10)
            b = r[i * 20]
            test(b)
            s = "D1." + (i * 20).to_s
            test(b.sb == s)
            if i == 0
                test(b.pb == nil)
            else
                test(b.pb == r[(i - 1) * 20])
            end
            d1 = b
            test(d1.is_a?(Test::D1))
            test(d1.sd1 == s)
            test(d1.pd1 == d1)
        end
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "base exception thrown as base exception... "
    STDOUT.flush
    begin
        t.throwBaseAsBase()
        test(false)
    rescue Test::BaseException => e
        test(e.ice_id() == "::Test::BaseException")
        test(e.sbe == "sbe")
        test(e.pb)
        test(e.pb.sb == "sb")
        test(e.pb.pb == e.pb)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "derived exception thrown as base exception... "
    STDOUT.flush
    begin
        t.throwDerivedAsBase()
        test(false)
    rescue Test::DerivedException => e
        test(e.ice_id() == "::Test::DerivedException")
        test(e.sbe == "sbe")
        test(e.pb)
        test(e.pb.sb == "sb1")
        test(e.pb.pb == e.pb)
        test(e.sde == "sde1")
        test(e.pd1)
        test(e.pd1.sb == "sb2")
        test(e.pd1.pb == e.pd1)
        test(e.pd1.sd1 == "sd2")
        test(e.pd1.pd1 == e.pd1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "derived exception thrown as derived exception... "
    STDOUT.flush
    begin
        t.throwDerivedAsDerived()
        test(false)
    rescue Test::DerivedException => e
        test(e.ice_id() == "::Test::DerivedException")
        test(e.sbe == "sbe")
        test(e.pb)
        test(e.pb.sb == "sb1")
        test(e.pb.pb == e.pb)
        test(e.sde == "sde1")
        test(e.pd1)
        test(e.pd1.sb == "sb2")
        test(e.pd1.pb == e.pd1)
        test(e.pd1.sd1 == "sd2")
        test(e.pd1.pd1 == e.pd1)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "unknown derived exception thrown as base exception... "
    STDOUT.flush
    begin
        t.throwUnknownDerivedAsBase()
        test(false)
    rescue Test::BaseException => e
        test(e.ice_id() == "::Test::BaseException")
        test(e.sbe == "sbe")
        test(e.pb)
        test(e.pb.sb == "sb d2")
        test(e.pb.pb == e.pb)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "forward-declared class... "
    STDOUT.flush
    begin
        f = t.useForward()
        test(f)
    rescue Ice::Exception
        test(false)
    end
    puts "ok"

    print "preserved classes... "
    STDOUT.flush

    #
    # Server knows the most-derived class PDerived.
    #
    pd = Test::PDerived.new()
    pd.pi = 3
    pd.ps = "preserved"
    pd.pb = pd

    r = t.exchangePBase(pd)
    test(r.is_a?(Test::PDerived))
    test(r.pi == 3)
    test(r.ps == "preserved")
    test(r.pb == r)

    #
    # Server only knows the base (non-preserved) type, so the object is sliced.
    #
    pu = Test::PCUnknown.new()
    pu.pi = 3
    pu.pu = "preserved"

    r = t.exchangePBase(pu)
    test(!r.is_a?(Test::PCUnknown))
    test(r.pi == 3)

    #
    # Server only knows the intermediate type Preserved. The object will be sliced to
    # Preserved for the 1.0 encoding; otherwise it should be returned intact.
    #
    pcd = Test::PCDerived.new()
    pcd.pi = 3
    pcd.pbs = [ pcd ]

    r = t.exchangePBase(pcd)
    if t.ice_getEncodingVersion() == Ice::Encoding_1_0
        test(!r.is_a?(Test::PCDerived))
        test(r.pi == 3)
    else
        test(r.is_a?(Test::PCDerived))
        test(r.pi == 3)
        test(r.pbs[0] == r)
    end

    #
    # Server only knows the intermediate type CompactPDerived. The object will be sliced to
    # CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
    #
    pcd = Test::CompactPCDerived.new()
    pcd.pi = 3
    pcd.pbs = [ pcd ]

    r = t.exchangePBase(pcd)
    if t.ice_getEncodingVersion() == Ice::Encoding_1_0
        test(!r.is_a?(Test::CompactPCDerived))
        test(r.pi == 3)
    else
        test(r.is_a?(Test::CompactPCDerived))
        test(r.pi == 3)
        test(r.pbs[0] == r)
    end

    #
    # Send an object that will have multiple preserved slices in the server.
    # The object will be sliced to PDerived for the 1.0 encoding.
    #
    pcd = Test::PCDerived3.new()
    pcd.pi = 3
    #
    # Sending more than 254 objects exercises the encoding for object ids.
    #
    pcd.pbs = []
    for i in (0...300)
        p2 = Test::PCDerived2.new()
        p2.pi = i
        p2.pbs = [ nil ] # Nil reference. This slice should not have an indirection table.
        p2.pcd2 = i
        pcd.pbs.push(p2)
    end
    pcd.pcd2 = pcd.pi
    pcd.pcd3 = pcd.pbs[10]

    r = t.exchangePBase(pcd)
    if t.ice_getEncodingVersion() == Ice::Encoding_1_0
        test(!r.is_a?(Test::PCDerived3))
        test(r.is_a?(Test::PDerived))
        test(r.pi == 3)
    else
        test(r.is_a?(Test::PCDerived3))
        test(r.pi == 3)
        for i in (0...300)
            p2 = r.pbs[i]
            test(p2.is_a?(Test::PCDerived2))
            test(p2.pi == i)
            test(p2.pbs.length == 1)
            test(!p2.pbs[0])
            test(p2.pcd2 == i)
        end
        test(r.pcd2 == r.pi)
        test(r.pcd3 == r.pbs[10])
    end

    #
    # Obtain an object with preserved slices and send it back to the server.
    # The preserved slices should be excluded for the 1.0 encoding, otherwise
    # they should be included.
    #
    p = t.PBSUnknownAsPreserved()
    t.checkPBSUnknown(p)
    if t.ice_getEncodingVersion() != Ice::Encoding_1_0
        slicedData = p.ice_getSlicedData()
        test(slicedData.slices.length == 1)
        test(slicedData.slices[0].typeId == "::Test::PSUnknown")
        t.ice_encodingVersion(Ice::Encoding_1_0).checkPBSUnknown(p)
    else
        test(!p.ice_getSlicedData())
    end

    #
    # Relay a graph through the server. This test uses a preserved class
    # with a class member.
    #
    c = Test::PNode.new()
    c._next = Test::PNode.new()
    c._next._next = Test::PNode.new()
    c._next._next._next = c    # Create a cyclic graph.

    n = t.exchangePNode(c)
    test(n._next != nil)
    test(n._next != n._next._next)
    test(n._next._next != n._next._next._next)
    test(n._next._next._next == n)
    n = nil        # Release reference.

    #
    # Obtain a preserved object from the server where the most-derived
    # type is unknown. The preserved slice refers to a graph of PNode
    # objects.
    #
    p = t.PBSUnknownAsPreservedWithGraph()
    test(p)
    t.checkPBSUnknownWithGraph(p)
    p = nil        # Release reference.

    #
    # Obtain a preserved object from the server where the most-derived
    # type is unknown. A data member in the preserved slice refers to the
    # outer object, so the chain of references looks like this:
    #
    # outer->slicedData->outer
    #
    p = t.PBSUnknown2AsPreservedWithGraph()
    test(p != nil)
    if t.ice_getEncodingVersion() != Ice::Encoding_1_0
        test(p._ice_slicedData != nil)
    end
    t.checkPBSUnknown2WithGraph(p)
    p._ice_slicedData = nil    # Break the cycle.
    p = nil                    # Release reference.

    puts "ok"

    return t
end
