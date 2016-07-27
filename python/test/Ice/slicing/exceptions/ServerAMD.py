#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
Ice.loadSlice('-I. --all ServerPrivateAMD.ice')
import Test

class TestI(Test.TestIntf):
    def shutdown_async(self, cb, current=None):
        current.adapter.getCommunicator().shutdown()
        cb.ice_response()

    def baseAsBase_async(self, cb, current=None):
        b = Test.Base()
        b.b = "Base.b"
        cb.ice_exception(b)

    def unknownDerivedAsBase_async(self, cb, current=None):
        d = Test.UnknownDerived()
        d.b = "UnknownDerived.b"
        d.ud = "UnknownDerived.ud"
        cb.ice_exception(d)

    def knownDerivedAsBase_async(self, cb, current=None):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        cb.ice_exception(d)

    def knownDerivedAsKnownDerived_async(self, cb, current=None):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        cb.ice_exception(d)

    def unknownIntermediateAsBase_async(self, cb, current=None):
        ui = Test.UnknownIntermediate()
        ui.b = "UnknownIntermediate.b"
        ui.ui = "UnknownIntermediate.ui"
        cb.ice_exception(ui)

    def knownIntermediateAsBase_async(self, cb, current=None):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        cb.ice_exception(ki)

    def knownMostDerivedAsBase_async(self, cb, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        cb.ice_exception(kmd)

    def knownIntermediateAsKnownIntermediate_async(self, cb, current=None):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        cb.ice_exception(ki)

    def knownMostDerivedAsKnownIntermediate_async(self, cb, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        cb.ice_exception(kmd)

    def knownMostDerivedAsKnownMostDerived_async(self, cb, current=None):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        cb.ice_exception(kmd)

    def unknownMostDerived1AsBase_async(self, cb, current=None):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        cb.ice_exception(umd1)

    def unknownMostDerived1AsKnownIntermediate_async(self, cb, current=None):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        cb.ice_exception(umd1)

    def unknownMostDerived2AsBase_async(self, cb, current=None):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        cb.ice_exception(umd2)

    def unknownMostDerived2AsBaseCompact_async(self, cb, current=None):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        cb.ice_exception(umd2)

    def knownPreservedAsBase_async(self, cb, r, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        cb.ice_exception(ex)

    def knownPreservedAsKnownPreserved_async(self, cb, r, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        cb.ice_exception(ex)

    def relayKnownPreservedAsBase_async(self, cb, r, current=None):
        try:
            r.knownPreservedAsBase()
            test(False)
        except Ice.Exception as ex:
            cb.ice_exception(ex)

    def relayKnownPreservedAsKnownPreserved_async(self, cb, r, current=None):
        try:
            r.knownPreservedAsKnownPreserved()
            test(False)
        except Ice.Exception as ex:
            cb.ice_exception(ex)

    def unknownPreservedAsBase_async(self, cb, r, current=None):
        ex = Test.SPreserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.SPreservedClass("bc", "spc")
        ex.p2 = ex.p1
        cb.ice_exception(ex)

    def unknownPreservedAsKnownPreserved_async(self, cb, r, current=None):
        ex = Test.SPreserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.SPreservedClass("bc", "spc")
        ex.p2 = ex.p1
        cb.ice_exception(ex)

    def relayUnknownPreservedAsBase_async(self, cb, r, current=None):
        try:
            r.unknownPreservedAsBase()
            test(False)
        except Ice.Exception as ex:
            cb.ice_exception(ex)

    def relayUnknownPreservedAsKnownPreserved_async(self, cb, r, current=None):
        try:
            r.unknownPreservedAsKnownPreserved()
            test(False)
        except Ice.Exception as ex:
            cb.ice_exception(ex)

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
