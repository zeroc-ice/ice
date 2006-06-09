#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, Twoways, TwowaysAMI

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, initData):
    print "testing stringToProxy...",
    ref = "test:default -p 12010 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)
    print "ok"

    print "testing ice_getCommunicator...",
    test(base.ice_getCommunicator() == communicator)
    print "ok"

    print "testing proxy methods... ",
    test(communicator.identityToString(base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()) == "other");
    test(base.ice_facet("facet").ice_getFacet() == "facet");
    test(base.ice_adapterId("id").ice_getAdapterId() == "id");
    test(base.ice_twoway().ice_isTwoway());
    test(base.ice_oneway().ice_isOneway());
    test(base.ice_batchOneway().ice_isBatchOneway());
    test(base.ice_datagram().ice_isDatagram());
    test(base.ice_batchDatagram().ice_isBatchDatagram());
    test(base.ice_secure(True).ice_getSecure());
    test(not base.ice_secure(False).ice_getSecure());
    print "ok"

    print "testing checked cast...",
    cl = Test.MyClassPrx.checkedCast(base)
    test(cl)

    derived = Test.MyDerivedClassPrx.checkedCast(cl)
    test(derived)
    test(cl == base)
    test(derived == base)
    test(cl == derived)

    loc = Ice.LocatorPrx.checkedCast(base)
    test(loc == None)

    #
    # Upcasting
    #
    cl2 = Test.MyClassPrx.checkedCast(derived)
    obj = Ice.ObjectPrx.checkedCast(derived)
    test(cl2)
    test(obj)
    test(cl2 == obj)
    test(cl2 == derived)

    print "ok"

    print "testing checked cast with context...",
    cref = "context:default -p 12010 -t 10000"
    cbase = communicator.stringToProxy(cref)
    test(cbase != None)

    tccp = Test.TestCheckedCastPrx.checkedCast(cbase)
    c = tccp.getContext()
    test(c == None or len(c) == 0)

    c = { }
    c["one"] = "hello"
    c["two"] =  "world"
    tccp = Test.TestCheckedCastPrx.checkedCast(cbase, c)
    c2 = tccp.getContext()
    test(c == c2)
    print "ok"

    print "testing twoway operations...",
    Twoways.twoways(communicator, initData, cl)
    Twoways.twoways(communicator, initData, derived)
    derived.opDerived()
    print "ok"

    print "testing twoway operations with AMI...",
    TwowaysAMI.twowaysAMI(communicator, initData, cl)
    TwowaysAMI.twowaysAMI(communicator, initData, derived)
    print "ok"

    return cl
