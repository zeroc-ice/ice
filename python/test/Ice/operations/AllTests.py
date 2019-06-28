#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice, Test, M, Twoways, TwowaysFuture, TwowaysAMI, Oneways, OnewaysFuture, OnewaysAMI, BatchOneways, sys
import BatchOnewaysAMI, BatchOnewaysFuture

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(helper, communicator):
    ref = "test:{0}".format(helper.getTestEndpoint())
    base = communicator.stringToProxy(ref)
    cl = Test.MyClassPrx.checkedCast(base)
    derived = Test.MyDerivedClassPrx.checkedCast(cl)
    bprx = M.BPrx.checkedCast(communicator.stringToProxy("b:{0}".format(helper.getTestEndpoint())))

    sys.stdout.write("testing twoway operations... ")
    sys.stdout.flush()
    Twoways.twoways(helper, cl, bprx)
    Twoways.twoways(helper, derived, bprx)
    derived.opDerived()
    print("ok")

    sys.stdout.write("testing oneway operations... ")
    sys.stdout.flush()
    Oneways.oneways(helper, cl)
    print("ok")

    sys.stdout.write("testing twoway operations with futures... ")
    sys.stdout.flush()
    TwowaysFuture.twowaysFuture(helper, cl)
    print("ok")

    sys.stdout.write("testing twoway operations with AMI... ")
    sys.stdout.flush()
    TwowaysAMI.twowaysAMI(helper, cl)
    print("ok")

    sys.stdout.write("testing oneway operations with futures... ")
    sys.stdout.flush()
    OnewaysFuture.onewaysFuture(helper, cl)
    print("ok")

    sys.stdout.write("testing oneway operations with AMI... ")
    sys.stdout.flush()
    OnewaysAMI.onewaysAMI(helper, cl)
    print("ok")

    sys.stdout.write("testing batch oneway operations...  ")
    sys.stdout.flush()
    BatchOneways.batchOneways(cl)
    BatchOneways.batchOneways(derived)
    print("ok")

    sys.stdout.write("testing batch oneway operations with futures...  ")
    sys.stdout.flush()
    BatchOnewaysFuture.batchOneways(cl)
    BatchOnewaysFuture.batchOneways(derived)
    print("ok")

    sys.stdout.write("testing batch oneway operations with AMI...  ")
    sys.stdout.flush()
    BatchOnewaysAMI.batchOneways(cl)
    BatchOnewaysAMI.batchOneways(derived)
    print("ok")

    sys.stdout.write("testing server shutdown... ")
    sys.stdout.flush()
    cl.shutdown()
    try:
        cl.ice_timeout(100).ice_ping()  # Use timeout to speed up testing on Windows
        test(False)
    except Ice.LocalException:
        print("ok")
