#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

require './Twoways'
require './BatchOneways'

def allTests(helper, communicator)
    ref = "test:#{helper.getTestEndpoint()}"
    base = communicator.stringToProxy(ref)
    cl = Test::MyClassPrx::checkedCast(base)
    derived = Test::MyDerivedClassPrx::checkedCast(cl)

    bprx = M::BPrx::checkedCast(communicator.stringToProxy("b:#{helper.getTestEndpoint()}"))

    print "testing twoway operations... "
    STDOUT.flush
    twoways(helper, communicator, cl, bprx)
    twoways(helper, communicator, derived, bprx)
    derived.opDerived()
    puts "ok"

    print "testing batch oneway operations... "
    STDOUT.flush
    batchOneways(cl)
    batchOneways(derived)
    puts "ok"

    return cl
end
