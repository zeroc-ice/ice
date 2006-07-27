#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Twoways'

def allTests(communicator, initData)
    print "testing stringToProxy... "
    STDOUT.flush
    ref = "test:default -p 12010 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)
    puts "ok"

    print "testing ice_getCommunicator... "
    STDOUT.flush
    test(base.ice_getCommunicator() == communicator)
    puts "ok"

    print "testing proxy methods... "
    test(communicator.identityToString(base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()) == "other");
    test(base.ice_facet("facet").ice_getFacet() == "facet");
    test(base.ice_adapterId("id").ice_getAdapterId() == "id");
    test(base.ice_twoway().ice_isTwoway());
    test(base.ice_oneway().ice_isOneway());
    test(base.ice_batchOneway().ice_isBatchOneway());
    test(base.ice_datagram().ice_isDatagram());
    test(base.ice_batchDatagram().ice_isBatchDatagram());
    test(base.ice_secure(true).ice_isSecure());
    test(!base.ice_secure(false).ice_isSecure());
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    cl = Test::MyClassPrx::checkedCast(base)
    test(cl)

    derived = Test::MyDerivedClassPrx::checkedCast(cl)
    test(derived)
    test(cl == base)
    test(derived == base)
    test(cl == derived)

    loc = Ice::LocatorPrx::checkedCast(base)
    test(loc == nil)

    #
    # Upcasting
    #
    cl2 = Test::MyClassPrx::checkedCast(derived)
    obj = Ice::ObjectPrx::checkedCast(derived)
    test(cl2)
    test(obj)
    test(cl2 == obj)
    test(cl2 == derived)

    puts "ok"

    print "testing checked cast with context... "
    STDOUT.flush
    cref = "context:default -p 12010 -t 10000"
    cbase = communicator.stringToProxy(cref)
    test(cbase != nil)

    tccp = Test::TestCheckedCastPrx::checkedCast(cbase)
    c = tccp.getContext()
    test(c == nil || c.length == 0)

    c = { }
    c["one"] = "hello"
    c["two"] =  "world"
    tccp = Test::TestCheckedCastPrx::checkedCast(cbase, c)
    c2 = tccp.getContext()
    test(c == c2)
    puts "ok"

    print "testing twoway operations... "
    STDOUT.flush
    twoways(communicator, initData, cl)
    twoways(communicator, initData, derived)
    derived.opDerived()
    puts "ok"

    return cl
end
