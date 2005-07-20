#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, os, sys, math

#
# Find Slice directory.
#
slice_dir = os.getenv('ICEPY_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, "slice")):
    slice_dir = os.getenv('ICE_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, "slice")):
    print sys.argv[0] + ': Slice directory not found. Define ICEPY_HOME or ICE_HOME.'
    sys.exit(1)

Ice.loadSlice('-I' + slice_dir + '/slice Test.ice')
import Test, Twoways, TwowaysAMI

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    print "testing stringToProxy... ",
    ref = "test:default -p 12345 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)
    print "ok"

    #print "testing ice_communicator... ",
    #test(base.ice_communicator() == communicator)
    #print "ok"

    print "testing checked cast... ",
    cl = Test.MyClassPrx.checkedCast(base)
    test(cl)

    derived = Test.MyDerivedClassPrx.checkedCast(cl)
    test(derived)
    test(cl == base)
    test(derived == base)
    test(cl == derived)

    loc = Ice.LocatorPrx.checkedCast(base)
    test(loc == None)

    #
    # Upcasting
    #
    cl2 = Test.MyClassPrx.checkedCast(derived)
    obj = Ice.ObjectPrx.checkedCast(derived)
    test(cl2)
    test(obj)
    test(cl2 == obj)
    test(cl2 == derived)

    print "ok"

    print "testing twoway operations... ",
    Twoways.twoways(cl)
    Twoways.twoways(derived)
    derived.opDerived()
    print "ok"

    print "testing twoway operations with AMI... ",
    TwowaysAMI.twowaysAMI(cl)
    TwowaysAMI.twowaysAMI(derived)
    print "ok"

    return cl
