#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice

Ice.loadSlice('Test.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    print "testing stringToProxy... ",
    ref = "initial:default -p 12345 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)
    print "ok"

    print "testing checked cast... ",
    initial = Test.InitialPrx.checkedCast(base)
    test(initial)
    test(initial == base)
    print "ok"

    print "getting B1... ",
    b1 = initial.getB1()
    test(b1)
    print "ok"
    
    print "getting B2... ",
    b2 = initial.getB2()
    test(b2)
    print "ok"
    
    print "getting C... ",
    c = initial.getC()
    test(c)
    print "ok"
    
    print "getting D... ",
    d = initial.getD()
    test(d)
    print "ok"
    
    print "checking consistency... ",
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theB == b1)
    test(b1.theC == None)
    test(isinstance(b1.theA, Test.B))
    test(b1.theA.theA == b1.theA)
    test(b1.theA.theB == b1)
    test(b1.theA.theC)
    test(b1.theA.theC.theB == b1.theA)
    test(b1.preMarshalInvoked)
    test(b1.postUnmarshalInvoked())
    test(b1.theA.preMarshalInvoked)
    test(b1.theA.postUnmarshalInvoked())
    test(b1.theA.theC.preMarshalInvoked)
    test(b1.theA.theC.postUnmarshalInvoked())
    # More tests possible for b2 and d, but I think this is already sufficient.
    test(b2.theA == b2)
    test(d.theC == None)
    print "ok"

    print "getting B1, B2, C, and D all at once... ",
    b1, b2, c, d = initial.getAll()
    test(b1)
    test(b2)
    test(c)
    test(d)
    print "ok"
    
    print "checking consistency... ",
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    test(b1.theA == b2)
    test(b1.theB == b1)
    test(b1.theC == None)
    test(b2.theA == b2)
    test(b2.theB == b1)
    test(b2.theC == c)
    test(c.theB == b2)
    test(d.theA == b1)
    test(d.theB == b2)
    test(d.theC == None)
    test(d.preMarshalInvoked)
    test(d.postUnmarshalInvoked())
    test(d.theA.preMarshalInvoked)
    test(d.theA.postUnmarshalInvoked())
    test(d.theB.preMarshalInvoked)
    test(d.theB.postUnmarshalInvoked())
    test(d.theB.theC.preMarshalInvoked)
    test(d.theB.theC.postUnmarshalInvoked())
    print "ok"

    return initial
