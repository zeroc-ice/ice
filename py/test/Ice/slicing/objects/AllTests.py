# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, sys

Ice.loadSlice('-I. --all Forward.ice ClientPrivate.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    obj = communicator.stringToProxy("Test:default -p 12345")
    t = Test.TestIntfPrx.checkedCast(obj)

    print "base as Object... ",
    o = None
    try:
        o = t.SBaseAsObject()
        test(o)
        test(o.ice_id() == "::Test::SBase")
    except Ice.Exception:
        test(False)
    sb = o
    test(isinstance(sb, Test.SBase))
    test(sb)
    test(sb.sb == "SBase.sb")
    print "ok"

    #print "base as Object (AMI)... ",
    #cb = AMI_Test_SBaseAsObjectI()
    #t.SBaseAsObject_async(cb)
    #test(cb.check())
    #print "ok"

    print "base as base... ",
    try:
        sb = t.SBaseAsSBase()
        test(sb.sb == "SBase.sb")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "base as base (AMI)... ",
    #cb = AMI_Test_SBaseAsSBaseI()
    #t.SBaseAsSBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "base with known derived as base... ",
    try:
        sb = t.SBSKnownDerivedAsSBase()
        test(sb.sb == "SBSKnownDerived.sb")
    except Ice.Exception:
        test(False)
    sbskd = sb
    test(isinstance(sbskd, Test.SBSKnownDerived))
    test(sbskd)
    test(sbskd.sbskd == "SBSKnownDerived.sbskd")
    print "ok"

    #print "base with known derived as base (AMI)... ",
    #cb = AMI_Test_SBSKnownDerivedAsSBaseI()
    #t.SBSKnownDerivedAsSBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "base with known derived as known derived... ",
    try:
        sbskd = t.SBSKnownDerivedAsSBSKnownDerived()
        test(sbskd.sbskd == "SBSKnownDerived.sbskd")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "base with known derived as known derived (AMI)... ",
    #cb = AMI_Test_SBSKnownDerivedAsSBSKnownDerivedI()
    #t.SBSKnownDerivedAsSBSKnownDerived_async(cb)
    #test(cb.check())
    #print "ok"

    print "base with unknown derived as base... ",
    try:
        sb = t.SBSUnknownDerivedAsSBase()
        test(sb.sb == "SBSUnknownDerived.sb")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "base with unknown derived as base (AMI)... ",
    #cb = AMI_Test_SBSUnknownDerivedAsSBaseI()
    #t.SBSUnknownDerivedAsSBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "unknown with Object as Object... ",
    try:
        o = t.SUnknownAsObject()
        test(o.ice_id() == "::Ice::Object")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "unknown with Object as Object (AMI)... ",
    #cb = AMI_Test_SUnknownAsObjectI()
    #t.SUnknownAsObject_async(cb)
    #test(cb.check())
    #print "ok"

    print "one-element cycle... ",
    try:
        b = t.oneElementCycle()
        test(b)
        test(b.ice_id() == "::Test::B")
        test(b.sb == "B1.sb")
        test(b.pb == b)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "one-element cycle (AMI)... ",
    #cb = AMI_Test_oneElementCycleI()
    #t.oneElementCycle_async(cb)
    #test(cb.check())
    #print "ok"

    print "two-element cycle... ",
    try:
        b1 = t.twoElementCycle()
        test(b1)
        test(b1.ice_id() == "::Test::B")
        test(b1.sb == "B1.sb")

        b2 = b1.pb
        test(b2)
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "B2.sb")
        test(b2.pb == b1)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "two-element cycle (AMI)... ",
    #cb = AMI_Test_twoElementCycleI()
    #t.twoElementCycle_async(cb)
    #test(cb.check())
    #print "ok"

    print "known derived pointer slicing as base... ",
    try:
        b1 = t.D1AsB()
        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb)
        test(b1.pb != b1)
        d1 = b1
        test(isinstance(d1, Test.D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1)
        test(d1.pd1 != b1)
        test(b1.pb == d1.pd1)

        b2 = b1.pb
        test(b2)
        test(b2.pb == b1)
        test(b2.sb == "D2.sb")
        test(b2.ice_id() == "::Test::B")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "known derived pointer slicing as base (AMI)... ",
    #cb = AMI_Test_D1AsBI()
    #t.D1AsB_async(cb)
    #test(cb.check())
    #print "ok"

    print "known derived pointer slicing as derived... ",
    try:
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
    except Ice.Exception:
        test(False)
    print "ok"

    #print "known derived pointer slicing as derived (AMI)... ",
    #cb = AMI_Test_D1AsD1I()
    #t.D1AsD1_async(cb)
    #test(cb.check())
    #print "ok"

    print "unknown derived pointer slicing as base... ",
    try:
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
        test(isinstance(d1, Test.D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "unknown derived pointer slicing as base (AMI)... ",
    #cb = AMI_Test_D2AsBI()
    #t.D2AsB_async(cb)
    #test(cb.check())
    #print "ok"

    print "param ptr slicing with known first... ",
    try:
        b1, b2 = t.paramTest1()

        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        test(isinstance(d1, Test.D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        test(b2)
        test(b2.ice_id() == "::Test::B")	# No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "param ptr slicing with known first (AMI)... ",
    #cb = AMI_Test_paramTest1I()
    #t.paramTest1_async(cb)
    #test(cb.check())
    #print "ok"

    print "param ptr slicing with unknown first... ",
    try:
        b2, b1 = t.paramTest2()

        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        test(isinstance(d1, Test.D1))
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        test(b2)
        test(b2.ice_id() == "::Test::B")	# No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
    except Ice.Exception:
        test(False)
    print "ok"

    print "return value identity with known first... ",
    try:
        r, p1, p2 = t.returnTest1()
        test(r == p1)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "return value identity with known first (AMI)... ",
    #cb = AMI_Test_returnTest1I()
    #t.returnTest1_async(cb)
    #test(cb.check())
    #print "ok"

    print "return value identity with unknown first... ",
    try:
        r, p1, p2 = t.returnTest2()
        test(r == p1)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "return value identity with unknown first (AMI)... ",
    #cb = AMI_Test_returnTest2I()
    #t.returnTest2_async(cb)
    #test(cb.check())
    #print "ok"

    print "return value identity for input params known first... ",
    try:
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = Test.D3()
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
        test(isinstance(p1, Test.D1))
        test(p1.sd1 == "D1.sd1")
        test(p1.pd1 == b1.pb)

        b2 = b1.pb
        test(b2)
        test(b2.sb == "D3.sb")
        test(b2.ice_id() == "::Test::B")	# Sliced by server
        test(b2.pb == b1)
        p3 = b2
        test(not isinstance(p3, Test.D3))

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "return value identity for input params known first (AMI)... ",
    #try:
    #    d1 = Test.D1()
    #    d1.sb = "D1.sb"
    #    d1.sd1 = "D1.sd1"
    #    d3 = Test.D3()
    #    d3.pb = d1
    #    d3.sb = "D3.sb"
    #    d3.sd3 = "D3.sd3"
    #    d3.pd3 = d1
    #    d1.pb = d3
    #    d1.pd1 = d3

    #    cb = AMI_Test_returnTest3I()
    #    t.returnTest3_async(cb, d1, d3)
    #    test(cb.check())
    #    b1 = cb.r

    #    test(b1)
    #    test(b1.sb == "D1.sb")
    #    test(b1.ice_id() == "::Test::D1")
    #    D1Ptr p1 = D1Ptr::dynamicCast(b1)
    #    test(p1)
    #    test(p1.sd1 == "D1.sd1")
    #    test(p1.pd1 == b1.pb)

    #    b2 = b1.pb
    #    test(b2)
    #    test(b2.sb == "D3.sb")
    #    test(b2.ice_id() == "::Test::B")	# Sliced by server
    #    test(b2.pb == b1)
    #    D3Ptr p3 = D3Ptr::dynamicCast(b2)
    #    test(!p3)

    #    test(b1 != d1)
    #    test(b1 != d3)
    #    test(b2 != d1)
    #    test(b2 != d3)
    #except Ice.Exception:
    #    test(False)
    #print "ok"

    print "return value identity for input params unknown first... ",
    try:
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = Test.D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        b1 = t.returnTest3(d3, d1)

        test(b1)
        test(b1.sb == "D3.sb")
        test(b1.ice_id() == "::Test::B")	# Sliced by server
        p1 = b1
        test(not isinstance(p1, Test.D3))

        b2 = b1.pb
        test(b2)
        test(b2.sb == "D1.sb")
        test(b2.ice_id() == "::Test::D1")
        test(b2.pb == b1)
        p3 = b2
        test(isinstance(p3, Test.D1))
        test(p3.sd1 == "D1.sd1")
        test(p3.pd1 == b1)

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "return value identity for input params unknown first (AMI)... ",
    #try:
    #    D1Ptr d1 = new D1
    #    d1.sb = "D1.sb"
    #    d1.sd1 = "D1.sd1"
    #    D3Ptr d3 = new D3
    #    d3.pb = d1
    #    d3.sb = "D3.sb"
    #    d3.sd3 = "D3.sd3"
    #    d3.pd3 = d1
    #    d1.pb = d3
    #    d1.pd1 = d3

    #    cb = AMI_Test_returnTest3I()
    #    t.returnTest3_async(cb, d3, d1)
    #    test(cb.check())
    #    BPtr b1 = cb.r

    #    test(b1)
    #    test(b1.sb == "D3.sb")
    #    test(b1.ice_id() == "::Test::B")	# Sliced by server
    #    D3Ptr p1 = D3Ptr::dynamicCast(b1)
    #    test(!p1)

    #    BPtr b2 = b1.pb
    #    test(b2)
    #    test(b2.sb == "D1.sb")
    #    test(b2.ice_id() == "::Test::D1")
    #    test(b2.pb == b1)
    #    D1Ptr p3 = D1Ptr::dynamicCast(b2)
    #    test(p3)
    #    test(p3.sd1 == "D1.sd1")
    #    test(p3.pd1 == b1)

    #    test(b1 != d1)
    #    test(b1 != d3)
    #    test(b2 != d1)
    #    test(b2 != d3)
    #except Ice.Exception:
    #    test(False)
    #print "ok"

    print "remainder unmarshaling (3 instances)... ",
    try:
        ret, p1, p2 = t.paramTest3()

        test(p1)
        test(p1.sb == "D2.sb (p1 1)")
        test(p1.pb == None)
        test(p1.ice_id() == "::Test::B")

        test(p2)
        test(p2.sb == "D2.sb (p2 1)")
        test(p2.pb == None)
        test(p2.ice_id() == "::Test::B")

        test(ret)
        test(ret.sb == "D1.sb (p2 2)")
        test(ret.pb == None)
        test(ret.ice_id() == "::Test::D1")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "remainder unmarshaling (3 instances) (AMI)... ",
    #cb = AMI_Test_paramTest3I()
    #t.paramTest3_async(cb)
    #test(cb.check())
    #print "ok"

    print "remainder unmarshaling (4 instances)... ",
    try:
        ret, b = t.paramTest4()

        test(b)
        test(b.sb == "D4.sb (1)")
        test(b.pb == None)
        test(b.ice_id() == "::Test::B")

        test(ret)
        test(ret.sb == "B.sb (2)")
        test(ret.pb == None)
        test(ret.ice_id() == "::Test::B")
    except Ice.Exception:
        test(False)
    print "ok"

    #print "remainder unmarshaling (4 instances) (AMI)... ",
    #cb = AMI_Test_paramTest4I()
    #t.paramTest4_async(cb)
    #test(cb.check())
    #print "ok"

    print "param ptr slicing, instance marshaled in unknown derived as base... ",
    try:
        b1 = Test.B()
        b1.sb = "B.sb(1)"
        b1.pb = b1

        d3 = Test.D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = b1

        b2 = Test.B()
        b2.sb = "B.sb(2)"
        b2.pb = b1

        r = t.returnTest3(d3, b2)

        test(r)
        test(r.ice_id() == "::Test::B")
        test(r.sb == "D3.sb")
        test(r.pb == r)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "param ptr slicing, instance marshaled in unknown derived as base (AMI)... ",
    #try:
    #    BPtr b1 = new B
    #    b1.sb = "B.sb(1)"
    #    b1.pb = b1

    #    D3Ptr d3 = new D3
    #    d3.sb = "D3.sb"
    #    d3.pb = d3
    #    d3.sd3 = "D3.sd3"
    #    d3.pd3 = b1

    #    BPtr b2 = new B
    #    b2.sb = "B.sb(2)"
    #    b2.pb = b1

    #    cb = AMI_Test_returnTest3I()
    #    t.returnTest3_async(cb, d3, b2)
    #    test(cb.check())
    #    BPtr r = cb.r

    #    test(r)
    #    test(r.ice_id() == "::Test::B")
    #    test(r.sb == "D3.sb")
    #    test(r.pb == r)
    #except Ice.Exception:
    #    test(False)
    #print "ok"

    print "param ptr slicing, instance marshaled in unknown derived as derived... ",
    try:
        d11 = Test.D1()
        d11.sb = "D1.sb(1)"
        d11.pb = d11
        d11.sd1 = "D1.sd1(1)"
        d11.pd1 = None

        d3 = Test.D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = d11

        d12 = Test.D1()
        d12.sb = "D1.sb(2)"
        d12.pb = d12
        d12.sd1 = "D1.sd1(2)"
        d12.pd1 = d11

        r = t.returnTest3(d3, d12)
        test(r)
        test(r.ice_id() == "::Test::B")
        test(r.sb == "D3.sb")
        test(r.pb == r)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ",
    #try:
    #    D1Ptr d11 = new D1
    #    d11.sb = "D1.sb(1)"
    #    d11.pb = d11
    #    d11.sd1 = "D1.sd1(1)"

    #    D3Ptr d3 = new D3
    #    d3.sb = "D3.sb"
    #    d3.pb = d3
    #    d3.sd3 = "D3.sd3"
    #    d3.pd3 = d11

    #    D1Ptr d12 = new D1
    #    d12.sb = "D1.sb(2)"
    #    d12.pb = d12
    #    d12.sd1 = "D1.sd1(2)"
    #    d12.pd1 = d11

    #    cb = AMI_Test_returnTest3I()
    #    t.returnTest3_async(cb, d3, d12)
    #    test(cb.check())
    #    BPtr r = cb.r
    #    test(r)
    #    test(r.ice_id() == "::Test::B")
    #    test(r.sb == "D3.sb")
    #    test(r.pb == r)
    #except Ice.Exception:
    #    test(False)
    #print "ok"

    print "sequence slicing... ",
    try:
        ss = Test.SS()
        ss1b = Test.B()
        ss1b.sb = "B.sb"
        ss1b.pb = ss1b

        ss1d1 = Test.D1()
        ss1d1.sb = "D1.sb"
        ss1d1.sd1 = "D1.sd1"
        ss1d1.pb = ss1b

        ss1d3 = Test.D3()
        ss1d3.sb = "D3.sb"
        ss1d3.sd3 = "D3.sd3"
        ss1d3.pb = ss1b

        ss2b = Test.B()
        ss2b.sb = "B.sb"
        ss2b.pb = ss1b

        ss2d1 = Test.D1()
        ss2d1.sb = "D1.sb"
        ss2d1.sd1 = "D1.sd1"
        ss2d1.pb = ss2b

        ss2d3 = Test.D3()
        ss2d3.sb = "D3.sb"
        ss2d3.sd3 = "D3.sd3"
        ss2d3.pb = ss2b

        ss1d1.pd1 = ss2b
        ss1d3.pd3 = ss2d1

        ss2d1.pd1 = ss1d3
        ss2d3.pd3 = ss1d1

        ss1 = Test.SS1()
        ss1.s = (ss1b, ss1d1, ss1d3)

        ss2 = Test.SS2()
        ss2.s = (ss2b, ss2d1, ss2d3)

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
    except Ice.Exception:
        test(False)
    print "ok"

    #print "sequence slicing (AMI)... ",
    #try:
    #    ss = Test.SS()
    #    BPtr ss1b = Test.B
    #    ss1b.sb = "B.sb"
    #    ss1b.pb = ss1b

    #    D1Ptr ss1d1 = Test.D1
    #    ss1d1.sb = "D1.sb"
    #    ss1d1.sd1 = "D1.sd1"
    #    ss1d1.pb = ss1b

    #    D3Ptr ss1d3 = Test.D3
    #    ss1d3.sb = "D3.sb"
    #    ss1d3.sd3 = "D3.sd3"
    #    ss1d3.pb = ss1b

    #    BPtr ss2b = Test.B
    #    ss2b.sb = "B.sb"
    #    ss2b.pb = ss1b

    #    D1Ptr ss2d1 = Test.D1
    #    ss2d1.sb = "D1.sb"
    #    ss2d1.sd1 = "D1.sd1"
    #    ss2d1.pb = ss2b

    #    D3Ptr ss2d3 = Test.D3
    #    ss2d3.sb = "D3.sb"
    #    ss2d3.sd3 = "D3.sd3"
    #    ss2d3.pb = ss2b

    #    ss1d1.pd1 = ss2b
    #    ss1d3.pd3 = ss2d1

    #    ss2d1.pd1 = ss1d3
    #    ss2d3.pd3 = ss1d1

    #    SS1Ptr ss1 = Test.SS1
    #    ss1.s.push_back(ss1b)
    #    ss1.s.push_back(ss1d1)
    #    ss1.s.push_back(ss1d3)

    #    SS2Ptr ss2 = Test.SS2
    #    ss2.s.push_back(ss2b)
    #    ss2.s.push_back(ss2d1)
    #    ss2.s.push_back(ss2d3)

    #    cb = AMI_Test_sequenceTestI()
    #    t.sequenceTest_async(cb, ss1, ss2)
    #    test(cb.check())
    #    ss = cb.r

    #    test(ss.c1)
    #    BPtr ss1b = ss.c1.s[0]
    #    BPtr ss1d1 = ss.c1.s[1]
    #    test(ss.c2)
    #    BPtr ss1d3 = ss.c1.s[2]

    #    test(ss.c2)
    #    BPtr ss2b = ss.c2.s[0]
    #    BPtr ss2d1 = ss.c2.s[1]
    #    BPtr ss2d3 = ss.c2.s[2]

    #    test(ss1b.pb == ss1b)
    #    test(ss1d1.pb == ss1b)
    #    test(ss1d3.pb == ss1b)

    #    test(ss2b.pb == ss1b)
    #    test(ss2d1.pb == ss2b)
    #    test(ss2d3.pb == ss2b)

    #    test(ss1b.ice_id() == "::Test::B")
    #    test(ss1d1.ice_id() == "::Test::D1")
    #    test(ss1d3.ice_id() == "::Test::B")

    #    test(ss2b.ice_id() == "::Test::B")
    #    test(ss2d1.ice_id() == "::Test::D1")
    #    test(ss2d3.ice_id() == "::Test::B")
    #except Ice.Exception:
    #    test(False)
    #print "ok"

    print "dictionary slicing... ",
    try:
        bin = {}
        for i in range(0, 10):
            d1 = Test.D1()
            s = "D1." + str(i)
            d1.sb = s
            d1.pb = d1
            d1.sd1 = s
            d1.pd1 = None
            bin[i] = d1

        r, bout = t.dictionaryTest(bin)

        test(len(bout) == 10)
        for i in range(0, 10):
            b = bout[i * 10]
            test(b)
            s = "D1." + str(i)
            test(b.sb == s)
            test(b.pb)
            test(b.pb != b)
            test(b.pb.sb == s)
            test(b.pb.pb == b.pb)

        test(len(r) == 10)
        for i in range(0, 10):
            b = r[i * 20]
            test(b)
            s = "D1." + str(i * 20)
            test(b.sb == s)
            if i == 0:
                test(b.pb == None)
            else:
                test(b.pb == r[(i - 1) * 20])
            d1 = b
            test(isinstance(d1, Test.D1))
            test(d1.sd1 == s)
            test(d1.pd1 == d1)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "dictionary slicing (AMI)... ",
    #try:
    #    BDict bin
    #    BDict bout
    #    BDict r
    #    int i
    #    for(i = 0 i < 10; ++i)
    #    {
    #        ostringstream s
    #        s << "D1." << i
    #        D1Ptr d1 = Test.D1
    #        d1.sb = s.str()
    #        d1.pb = d1
    #        d1.sd1 = s.str()
    #        bin[i] = d1
    #    }

    #    cb = AMI_Test_dictionaryTestI()
    #    t.dictionaryTest_async(cb, bin)
    #    test(cb.check())
    #    bout = cb.bout
    #    r = cb.r

    #    test(bout.size() == 10)
    #    for(i = 0 i < 10; ++i)
    #    {
    #        BPtr b = bout.find(i * 10).second
    #        test(b)
    #        std::ostringstream s
    #        s << "D1." << i
    #        test(b.sb == s.str())
    #        test(b.pb)
    #        test(b.pb != b)
    #        test(b.pb.sb == s.str())
    #        test(b.pb.pb == b.pb)
    #    }

    #    test(r.size() == 10)
    #    for(i = 0 i < 10; ++i)
    #    {
    #        BPtr b = r.find(i * 20).second
    #        test(b)
    #        std::ostringstream s
    #        s << "D1." << i * 20
    #        test(b.sb == s.str())
    #        test(b.pb == (i == 0 ? BPtr(0) : r.find((i - 1) * 20).second))
    #        D1Ptr d1 = D1Ptr::dynamicCast(b)
    #        test(d1)
    #        test(d1.sd1 == s.str())
    #        test(d1.pd1 == d1)
    #    }
    #except Ice.Exception:
    #    test(False)
    #print "ok"

    print "base exception thrown as base exception... ",
    try:
        t.throwBaseAsBase()
        test(False)
    except Test.BaseException, e:
        test(e.ice_id() == "::Test::BaseException")
        test(e.sbe == "sbe")
        test(e.pb)
        test(e.pb.sb == "sb")
        test(e.pb.pb == e.pb)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "base exception thrown as base exception (AMI)... ",
    #cb = AMI_Test_throwBaseAsBaseI()
    #t.throwBaseAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "derived exception thrown as base exception... ",
    try:
        t.throwDerivedAsBase()
        test(False)
    except Test.DerivedException, e:
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
    except Ice.Exception:
        test(False)
    print "ok"

    #print "derived exception thrown as base exception (AMI)... ",
    #cb = AMI_Test_throwDerivedAsBaseI()
    #t.throwDerivedAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "derived exception thrown as derived exception... ",
    try:
        t.throwDerivedAsDerived()
        test(False)
    except Test.DerivedException, e:
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
    except Ice.Exception:
        test(False)
    print "ok"

    #print "derived exception thrown as derived exception (AMI)... ",
    #cb = AMI_Test_throwDerivedAsDerivedI()
    #t.throwDerivedAsDerived_async(cb)
    #test(cb.check())
    #print "ok"

    print "unknown derived exception thrown as base exception... ",
    try:
        t.throwUnknownDerivedAsBase()
        test(False)
    except Test.BaseException, e:
        test(e.ice_id() == "::Test::BaseException")
        test(e.sbe == "sbe")
        test(e.pb)
        test(e.pb.sb == "sb d2")
        test(e.pb.pb == e.pb)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "unknown derived exception thrown as base exception (AMI)... ",
    #cb = AMI_Test_throwUnknownDerivedAsBaseI()
    #t.throwUnknownDerivedAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "forward-declared class... ",
    try:
        f = t.useForward()
        test(f)
    except Ice.Exception:
        test(False)
    print "ok"

    #print "forward-declared class (AMI)... ",
    #cb = AMI_Test_useForwardI()
    #t.useForward_async(cb)
    #test(cb.check())
    #print "ok"

    return t
