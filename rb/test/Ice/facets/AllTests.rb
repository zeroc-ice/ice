#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(communicator)
    print "testing stringToProxy... "
    STDOUT.flush
    ref = "d:default -p 12010 -t 10000"
    db = communicator.stringToProxy(ref)
    test(db)
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    d = Test::DPrx::checkedCast(db)
    test(d)
    test(d == db)
    puts "ok"

    print "testing non-facets A, B, C, and D... "
    STDOUT.flush
    test(d.callA() == "A")
    test(d.callB() == "B")
    test(d.callC() == "C")
    test(d.callD() == "D")
    puts "ok"

    print "testing facets A, B, C, and D... "
    STDOUT.flush
    df = Test::DPrx::checkedCast(d, "facetABCD")
    test(df)
    test(df.callA() == "A")
    test(df.callB() == "B")
    test(df.callC() == "C")
    test(df.callD() == "D")
    puts "ok"

    print "testing facets E and F... "
    STDOUT.flush
    ff = Test::FPrx::checkedCast(d, "facetEF")
    test(ff)
    test(ff.callE() == "E")
    test(ff.callF() == "F")
    puts "ok"

    print "testing facet G... "
    STDOUT.flush
    gf = Test::GPrx::checkedCast(ff, "facetGH")
    test(gf)
    test(gf.callG() == "G")
    puts "ok"

    print "testing whether casting preserves the facet... "
    STDOUT.flush
    hf = Test::HPrx::checkedCast(gf)
    test(hf)
    test(hf.callG() == "G")
    test(hf.callH() == "H")
    puts "ok"

    return gf
end
