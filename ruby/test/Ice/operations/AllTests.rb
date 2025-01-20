# Copyright (c) ZeroC, Inc.

require './Twoways'
require './BatchOneways'

def allTests(helper, communicator)
    ref = "test:#{helper.getTestEndpoint()}"
    cl = Test::MyClassPrx.new(communicator, ref)
    derived = Test::MyDerivedClassPrx.checkedCast(cl)

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
