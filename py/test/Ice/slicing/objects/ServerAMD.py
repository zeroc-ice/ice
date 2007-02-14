#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.insert(0, os.path.join(toplevel, "python"))
sys.path.insert(0, os.path.join(toplevel, "lib"))

import Ice
Ice.loadSlice('-I. --all ServerPrivateAMD.ice Forward.ice')
import Test

class TestI(Test.TestIntf):
    def __init__(self, adapter):
        self._adapter = adapter

    def SBaseAsObject_async(self, cb, current=None):
        sb = Test.SBase()
        sb.sb = "SBase.sb"
        cb.ice_response(sb)

    def SBaseAsSBase_async(self, cb, current=None):
        sb = Test.SBase()
        sb.sb = "SBase.sb"
        cb.ice_response(sb)

    def SBSKnownDerivedAsSBase_async(self, cb, current=None):
        sbskd = Test.SBSKnownDerived()
        sbskd.sb = "SBSKnownDerived.sb"
        sbskd.sbskd = "SBSKnownDerived.sbskd"
        cb.ice_response(sbskd)

    def SBSKnownDerivedAsSBSKnownDerived_async(self, cb, current=None):
        sbskd = Test.SBSKnownDerived()
        sbskd.sb = "SBSKnownDerived.sb"
        sbskd.sbskd = "SBSKnownDerived.sbskd"
        cb.ice_response(sbskd)

    def SBSUnknownDerivedAsSBase_async(self, cb, current=None):
        sbsud = Test.SBSUnknownDerived()
        sbsud.sb = "SBSUnknownDerived.sb"
        sbsud.sbsud = "SBSUnknownDerived.sbsud"
        cb.ice_response(sbsud)

    def SUnknownAsObject_async(self, cb, current=None):
        su = Test.SUnknown()
        su.su = "SUnknown.su"
        cb.ice_response(su)

    def oneElementCycle_async(self, cb, current=None):
        b = Test.B()
        b.sb = "B1.sb"
        b.pb = b
        cb.ice_response(b)

    def twoElementCycle_async(self, cb, current=None):
        b1 = Test.B()
        b1.sb = "B1.sb"
        b2 = Test.B()
        b2.sb = "B2.sb"
        b2.pb = b1
        b1.pb = b2
        cb.ice_response(b1)

    def D1AsB_async(self, cb, current=None):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        cb.ice_response(d1)

    def D1AsD1_async(self, cb, current=None):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        cb.ice_response(d1)

    def D2AsB_async(self, cb, current=None):
        d2 = Test.D2()
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d1 = Test.D1()
        d1.pb = d2
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d1.pd1 = d2
        d2.pb = d1
        d2.pd2 = d1
        cb.ice_response(d2)

    def paramTest1_async(self, cb, current=None):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        cb.ice_response(d1, d2)

    def paramTest2_async(self, cb, current=None):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        cb.ice_response(d2, d1)

    def paramTest3_async(self, cb, current=None):
        d2 = Test.D2()
        d2.sb = "D2.sb (p1 1)"
        d2.pb = None
        d2.sd2 = "D2.sd2 (p1 1)"

        d1 = Test.D1()
        d1.sb = "D1.sb (p1 2)"
        d1.pb = None
        d1.sd1 = "D1.sd2 (p1 2)"
        d1.pd1 = None
        d2.pd2 = d1

        d4 = Test.D2()
        d4.sb = "D2.sb (p2 1)"
        d4.pb = None
        d4.sd2 = "D2.sd2 (p2 1)"

        d3 = Test.D1()
        d3.sb = "D1.sb (p2 2)"
        d3.pb = None
        d3.sd1 = "D1.sd2 (p2 2)"
        d3.pd1 = None
        d4.pd2 = d3

        cb.ice_response(d3, d2, d4)

    def paramTest4_async(self, cb, current=None):
        d4 = Test.D4()
        d4.sb = "D4.sb (1)"
        d4.pb = None
        d4.p1 = Test.B()
        d4.p1.sb = "B.sb (1)"
        d4.p1.pb = None
        d4.p2 = Test.B()
        d4.p2.sb = "B.sb (2)"
        d4.p2.pb = None
        cb.ice_response(d4.p2, d4)

    def returnTest1_async(self, cb, current=None):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        cb.ice_response(d2, d2, d1)

    def returnTest2_async(self, cb, current=None):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        cb.ice_response(d1, d1, d2)

    def returnTest3_async(self, cb, p1, p2, current=None):
        cb.ice_response(p1)

    def sequenceTest_async(self, cb, p1, p2, current=None):
        ss = Test.SS()
        ss.c1 = p1
        ss.c2 = p2
        cb.ice_response(ss)

    def dictionaryTest_async(self, cb, bin, current=None):
        bout = {}
        for i in range(0, 10):
            b = bin[i]
            d2 = Test.D2()
            d2.sb = b.sb
            d2.pb = b.pb
            d2.sd2 = "D2"
            d2.pd2 = d2
            bout[i * 10] = d2

        r = {}
        for i in range(0, 10):
            s = "D1." + str(i * 20)
            d1 = Test.D1()
            d1.sb = s
            if i == 0:
                d1.pb = None
            else:
                d1.pb = r[(i - 1) * 20]
            d1.sd1 = s
            d1.pd1 = d1
            r[i * 20] = d1

        cb.ice_response(r, bout)

    def throwBaseAsBase_async(self, cb, current=None):
        be = Test.BaseException()
        be.sbe = "sbe"
        be.pb = Test.B()
        be.pb.sb = "sb"
        be.pb.pb = be.pb
        cb.ice_exception(be)

    def throwDerivedAsBase_async(self, cb, current=None):
        de = Test.DerivedException()
        de.sbe = "sbe"
        de.pb = Test.B()
        de.pb.sb = "sb1"
        de.pb.pb = de.pb
        de.sde = "sde1"
        de.pd1 = Test.D1()
        de.pd1.sb = "sb2"
        de.pd1.pb = de.pd1
        de.pd1.sd1 = "sd2"
        de.pd1.pd1 = de.pd1
        cb.ice_exception(de)

    def throwDerivedAsDerived_async(self, cb, current=None):
        de = Test.DerivedException()
        de.sbe = "sbe"
        de.pb = Test.B()
        de.pb.sb = "sb1"
        de.pb.pb = de.pb
        de.sde = "sde1"
        de.pd1 = Test.D1()
        de.pd1.sb = "sb2"
        de.pd1.pb = de.pd1
        de.pd1.sd1 = "sd2"
        de.pd1.pd1 = de.pd1
        cb.ice_exception(de)

    def throwUnknownDerivedAsBase_async(self, cb, current=None):
        d2 = Test.D2()
        d2.sb = "sb d2"
        d2.pb = d2
        d2.sd2 = "sd2 d2"
        d2.pd2 = d2

        ude = Test.UnknownDerivedException()
        ude.sbe = "sbe"
        ude.pb = d2
        ude.sude = "sude"
        ude.pd2 = d2
        cb.ice_exception(ude)

    def useForward_async(self, cb, current=None):
        f = Test.Forward()
        f.h = Test.Hidden()
        f.h.f = f
        cb.ice_response(f)

    def shutdown_async(self, cb, current=None):
        self._adapter.getCommunicator().shutdown()
        cb.ice_response()

def run(args, communicator):
    properties = communicator.getProperties()
    properties.setProperty("Ice.Warn.Dispatch", "0")
    properties.setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = TestI(adapter)
    adapter.add(object, Ice.stringToIdentity("Test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    communicator = Ice.initialize(sys.argv)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
