# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, Twoways, TwowaysAMI, TwowaysNewAMI, Oneways, OnewaysAMI, OnewaysNewAMI, BatchOneways

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, collocated):
    ref = "test:default -p 12010"
    base = communicator.stringToProxy(ref)
    cl = Test.MyClassPrx.checkedCast(base)
    derived = Test.MyDerivedClassPrx.checkedCast(cl)

    print "testing twoway operations...",
    Twoways.twoways(communicator, cl)
    Twoways.twoways(communicator, derived)
    derived.opDerived()
    print "ok"

    print "testing oneway operations...",
    Oneways.oneways(communicator, cl)
    print "ok"

    if not collocated:
        print "testing twoway operations with AMI...",
        TwowaysAMI.twowaysAMI(communicator, cl)
        print "ok"

        print "testing twoway operations with new AMI mapping...",
        TwowaysNewAMI.twowaysNewAMI(communicator, cl)
        print "ok"

        print "testing oneway operations with AMI...",
        OnewaysAMI.onewaysAMI(communicator, cl)
        print "ok"

        print "testing oneway operations with new AMI mapping...",
        OnewaysNewAMI.onewaysNewAMI(communicator, cl)
        print "ok"

        print "testing batch oneway operations... ",
        BatchOneways.batchOneways(cl)
        BatchOneways.batchOneways(derived)
        print "ok"

    return cl
