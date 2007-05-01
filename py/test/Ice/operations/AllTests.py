#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, Twoways, TwowaysAMI, BatchOneways

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, collocated):
    ref = "test:default -p 12010 -t 10000"
    base = communicator.stringToProxy(ref)
    cl = Test.MyClassPrx.checkedCast(base)
    derived = Test.MyDerivedClassPrx.checkedCast(cl)

    print "testing twoway operations...",
    Twoways.twoways(communicator, cl)
    Twoways.twoways(communicator, derived)
    derived.opDerived()
    print "ok"

    print "testing twoway operations with AMI...",
    TwowaysAMI.twowaysAMI(communicator, cl)
    TwowaysAMI.twowaysAMI(communicator, derived)
    print "ok"

    if not collocated:
        print "testing batch oneway operations... ",
        BatchOneways.batchOneways(cl)
        BatchOneways.batchOneways(derived)
        print "ok"

    return cl
