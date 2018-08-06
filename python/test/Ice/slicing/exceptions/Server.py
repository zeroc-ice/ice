#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("-I. --all ServerPrivate.ice")
import Ice
import Test


class TestI(Test.TestIntf):
    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def baseAsBase(self, current=None):
        b = Test.Base()
        b.b = "Base.b"
        raise b

    def unknownDerivedAsBase(self, current=None):
        d = Test.UnknownDerived()
        d.b = "UnknownDerived.b"
        d.ud = "UnknownDerived.ud"
        raise d

    def knownDerivedAsBase(self, current=None):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        raise d

    def knownDerivedAsKnownDerived(self, current=None):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        raise d

    def unknownIntermediateAsBase(self, current=None):
        ui = Test.UnknownIntermediate()
        ui.b = "UnknownIntermediate.b"
        ui.ui = "UnknownIntermediate.ui"
        raise ui

    def knownIntermediateAsBase(self, current=None):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        raise ki

    def knownMostDerivedAsBase(self, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        raise kmd

    def knownIntermediateAsKnownIntermediate(self, current=None):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        raise ki

    def knownMostDerivedAsKnownIntermediate(self, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        raise kmd

    def knownMostDerivedAsKnownMostDerived(self, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        raise kmd

    def unknownMostDerived1AsBase(self, current=None):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        raise umd1

    def unknownMostDerived1AsKnownIntermediate(self, current=None):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        raise umd1

    def unknownMostDerived2AsBase(self, current=None):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        raise umd2

    def unknownMostDerived2AsBaseCompact(self, current=None):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        raise umd2

    def knownPreservedAsBase(self, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        raise ex

    def knownPreservedAsKnownPreserved(self, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        raise ex

    def relayKnownPreservedAsBase(self, r, current):
        p = Test.RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()))
        p.knownPreservedAsBase()

    def relayKnownPreservedAsKnownPreserved(self, r, current):
        p = Test.RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()))
        p.knownPreservedAsKnownPreserved()

    def unknownPreservedAsBase(self, current=None):
        ex = Test.SPreserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.SPreservedClass("bc", "spc")
        ex.p2 = ex.p1
        raise ex

    def unknownPreservedAsKnownPreserved(self, current=None):
        ex = Test.SPreserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.SPreservedClass("bc", "spc")
        ex.p2 = ex.p1
        raise ex

    def relayUnknownPreservedAsBase(self, r, current):
        p = Test.RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()))
        p.unknownPreservedAsBase()

    def relayUnknownPreservedAsKnownPreserved(self, r, current):
        p = Test.RelayPrx.uncheckedCast(current.con.createProxy(r.ice_getIdentity()))
        p.unknownPreservedAsKnownPreserved()


class Server(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                     "{0} -t 10000".format(self.getTestEndpoint()))
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("Test"))
            adapter.activate()
            communicator.waitForShutdown()
