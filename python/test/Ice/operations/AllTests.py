# Copyright (c) ZeroC, Inc.

import Ice
import Test
import Twoways
import TwowaysFuture
import Oneways
import OnewaysFuture
import BatchOneways
import sys
import BatchOnewaysFuture


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper, communicator):
    cl = Test.MyClassPrx(communicator, f"test:{helper.getTestEndpoint()}")
    derived = Test.MyDerivedClassPrx(communicator, f"test:{helper.getTestEndpoint()}")

    sys.stdout.write("testing twoway operations... ")
    sys.stdout.flush()
    Twoways.twoways(helper, cl)
    Twoways.twoways(helper, derived)
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

    sys.stdout.write("testing oneway operations with futures... ")
    sys.stdout.flush()
    OnewaysFuture.onewaysFuture(helper, cl)
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

    sys.stdout.write("testing server shutdown... ")
    sys.stdout.flush()
    cl.shutdown()
    try:
        cl.ice_invocationTimeout(100).ice_ping()  # Use timeout to speed up testing on Windows
        test(False)
    except Ice.LocalException:
        print("ok")
