# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, Twoways, TwowaysFuture, TwowaysAMI, Oneways, OnewaysFuture, OnewaysAMI, BatchOneways, sys
import BatchOnewaysAMI, BatchOnewaysFuture

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(helper, communicator):
    ref = "test:{0}".format(helper.getTestEndpoint())
    base = communicator.stringToProxy(ref)
    cl = Test.MyClassPrx.checkedCast(base)
    derived = Test.MyDerivedClassPrx.checkedCast(cl)

    sys.stdout.write("testing twoway operations... ")
    sys.stdout.flush()
    Twoways.twoways(communicator, cl)
    Twoways.twoways(communicator, derived)
    derived.opDerived()
    print("ok")

    sys.stdout.write("testing oneway operations... ")
    sys.stdout.flush()
    Oneways.oneways(communicator, cl)
    print("ok")

    sys.stdout.write("testing twoway operations with futures... ")
    sys.stdout.flush()
    TwowaysFuture.twowaysFuture(communicator, cl)
    print("ok")

    sys.stdout.write("testing twoway operations with AMI... ")
    sys.stdout.flush()
    TwowaysAMI.twowaysAMI(communicator, cl)
    print("ok")

    sys.stdout.write("testing oneway operations with futures... ")
    sys.stdout.flush()
    OnewaysFuture.onewaysFuture(communicator, cl)
    print("ok")

    sys.stdout.write("testing oneway operations with AMI... ")
    sys.stdout.flush()
    OnewaysAMI.onewaysAMI(communicator, cl)
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
