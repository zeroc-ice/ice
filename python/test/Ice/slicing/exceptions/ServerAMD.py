#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
Ice.loadSlice('-I. --all ServerPrivate.ice')
import Test

class TestI(Test.TestIntf):
    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def baseAsBase(self, current=None):
        b = Test.Base()
        b.b = "Base.b"
        f = Ice.Future()
        f.set_exception(b)
        return f

    def unknownDerivedAsBase(self, current=None):
        d = Test.UnknownDerived()
        d.b = "UnknownDerived.b"
        d.ud = "UnknownDerived.ud"
        f = Ice.Future()
        f.set_exception(d)
        return f

    def knownDerivedAsBase(self, current=None):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        f = Ice.Future()
        f.set_exception(d)
        return f

    def knownDerivedAsKnownDerived(self, current=None):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        f = Ice.Future()
        f.set_exception(d)
        return f

    def unknownIntermediateAsBase(self, current=None):
        ui = Test.UnknownIntermediate()
        ui.b = "UnknownIntermediate.b"
        ui.ui = "UnknownIntermediate.ui"
        f = Ice.Future()
        f.set_exception(ui)
        return f

    def knownIntermediateAsBase(self, current=None):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        f = Ice.Future()
        f.set_exception(ki)
        return f

    def knownMostDerivedAsBase(self, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        f = Ice.Future()
        f.set_exception(kmd)
        return f

    def knownIntermediateAsKnownIntermediate(self, current=None):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        f = Ice.Future()
        f.set_exception(ki)
        return f

    def knownMostDerivedAsKnownIntermediate(self, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        f = Ice.Future()
        f.set_exception(kmd)
        return f

    def knownMostDerivedAsKnownMostDerived(self, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        f = Ice.Future()
        f.set_exception(kmd)
        return f

    def unknownMostDerived1AsBase(self, current=None):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        f = Ice.Future()
        f.set_exception(umd1)
        return f

    def unknownMostDerived1AsKnownIntermediate(self, current=None):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        f = Ice.Future()
        f.set_exception(umd1)
        return f

    def unknownMostDerived2AsBase(self, current=None):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        f = Ice.Future()
        f.set_exception(umd2)
        return f

    def unknownMostDerived2AsBaseCompact(self, current=None):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        f = Ice.Future()
        f.set_exception(umd2)
        return f

    def knownPreservedAsBase(self, r, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def knownPreservedAsKnownPreserved(self, r, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def relayKnownPreservedAsBase(self, r, current=None):
        f = Ice.Future()
        try:
            r.knownPreservedAsBase()
            test(False)
        except Ice.Exception as ex:
            f.set_exception(ex)
        return f

    def relayKnownPreservedAsKnownPreserved(self, r, current=None):
        f = Ice.Future()
        try:
            r.knownPreservedAsKnownPreserved()
            test(False)
        except Ice.Exception as ex:
            f.set_exception(ex)
        return f

    def unknownPreservedAsBase(self, r, current=None):
        ex = Test.SPreserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.SPreservedClass("bc", "spc")
        ex.p2 = ex.p1
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def unknownPreservedAsKnownPreserved(self, r, current=None):
        ex = Test.SPreserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.SPreservedClass("bc", "spc")
        ex.p2 = ex.p1
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def relayUnknownPreservedAsBase(self, r, current=None):
        f = Ice.Future()
        try:
            r.unknownPreservedAsBase()
            test(False)
        except Ice.Exception as ex:
            f.set_exception(ex)
        return f

    def relayUnknownPreservedAsKnownPreserved(self, r, current=None):
        f = Ice.Future()
        try:
            r.unknownPreservedAsKnownPreserved()
            test(False)
        except Ice.Exception as ex:
            f.set_exception(ex)
        return f

def run(args, communicator):
    properties = communicator.getProperties()
    properties.setProperty("Ice.Warn.Dispatch", "0")
    properties.setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = TestI()
    adapter.add(object, Ice.stringToIdentity("Test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    with Ice.initialize(sys.argv) as communicator:
         status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
