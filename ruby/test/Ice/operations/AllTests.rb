# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require './Twoways'
require './BatchOneways'

def allTests(communicator)
    ref = "test:default -p 12010"
    base = communicator.stringToProxy(ref)
    cl = Test::MyClassPrx::checkedCast(base)
    derived = Test::MyDerivedClassPrx::checkedCast(cl)

    print "testing twoway operations... "
    STDOUT.flush
    twoways(communicator, cl)
    twoways(communicator, derived)
    derived.opDerived()
    puts "ok"

    print "testing batch oneway operations... "
    STDOUT.flush
    batchOneways(cl)
    batchOneways(derived)
    puts "ok"

    return cl
end
