#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

TestHelper.loadSlice("-I. --all ServerPrivate.ice")
import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestI(Test.TestIntf):
    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

    def baseAsBase(self, current):
        b = Test.Base()
        b.b = "Base.b"
        f = Ice.Future()
        f.set_exception(b)
        return f

    def unknownDerivedAsBase(self, current):
        d = Test.UnknownDerived()
        d.b = "UnknownDerived.b"
        d.ud = "UnknownDerived.ud"
        f = Ice.Future()
        f.set_exception(d)
        return f

    def knownDerivedAsBase(self, current):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        f = Ice.Future()
        f.set_exception(d)
        return f

    def knownDerivedAsKnownDerived(self, current):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        f = Ice.Future()
        f.set_exception(d)
        return f

    def unknownIntermediateAsBase(self, current):
        ui = Test.UnknownIntermediate()
        ui.b = "UnknownIntermediate.b"
        ui.ui = "UnknownIntermediate.ui"
        f = Ice.Future()
        f.set_exception(ui)
        return f

    def knownIntermediateAsBase(self, current):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        f = Ice.Future()
        f.set_exception(ki)
        return f

    def knownMostDerivedAsBase(self, current):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        f = Ice.Future()
        f.set_exception(kmd)
        return f

    def knownIntermediateAsKnownIntermediate(self, current):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        f = Ice.Future()
        f.set_exception(ki)
        return f

    def knownMostDerivedAsKnownIntermediate(self, current):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        f = Ice.Future()
        f.set_exception(kmd)
        return f

    def knownMostDerivedAsKnownMostDerived(self, current):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        f = Ice.Future()
        f.set_exception(kmd)
        return f

    def unknownMostDerived1AsBase(self, current):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        f = Ice.Future()
        f.set_exception(umd1)
        return f

    def unknownMostDerived1AsKnownIntermediate(self, current):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        f = Ice.Future()
        f.set_exception(umd1)
        return f

    def unknownMostDerived2AsBase(self, current):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        f = Ice.Future()
        f.set_exception(umd2)
        return f


class ServerAMD(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", "{0} -t 10000".format(self.getTestEndpoint())
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("Test"))
            adapter.activate()
            communicator.waitForShutdown()
