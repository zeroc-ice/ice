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

    print "testing twoway operations... "
    STDOUT.flush
    twoways(helper, communicator, cl)
    twoways(helper, communicator, derived)
    derived.opDerived()
    puts "ok"

    print "testing batch oneway operations... "
    STDOUT.flush
    batchOneways(cl)
    batchOneways(derived)
    puts "ok"

    return cl
end
