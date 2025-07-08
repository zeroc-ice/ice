#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper
import Ice
import Test


class TestI(Test.TestIntf):
    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

    def baseAsBase(self, current):
        b = Test.Base()
        b.b = "Base.b"
        raise b

    def unknownDerivedAsBase(self, current):
        d = Test.UnknownDerived()
        d.b = "UnknownDerived.b"
        d.ud = "UnknownDerived.ud"
        raise d

    def knownDerivedAsBase(self, current):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        raise d

    def knownDerivedAsKnownDerived(self, current):
        d = Test.KnownDerived()
        d.b = "KnownDerived.b"
        d.kd = "KnownDerived.kd"
        raise d

    def unknownIntermediateAsBase(self, current):
        ui = Test.UnknownIntermediate()
        ui.b = "UnknownIntermediate.b"
        ui.ui = "UnknownIntermediate.ui"
        raise ui

    def knownIntermediateAsBase(self, current):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        raise ki

    def knownMostDerivedAsBase(self, current):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        raise kmd

    def knownIntermediateAsKnownIntermediate(self, current):
        ki = Test.KnownIntermediate()
        ki.b = "KnownIntermediate.b"
        ki.ki = "KnownIntermediate.ki"
        raise ki

    def knownMostDerivedAsKnownIntermediate(self, current):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        raise kmd

    def knownMostDerivedAsKnownMostDerived(self, current):
        kmd = Test.KnownMostDerived()
        kmd.b = "KnownMostDerived.b"
        kmd.ki = "KnownMostDerived.ki"
        kmd.kmd = "KnownMostDerived.kmd"
        raise kmd

    def unknownMostDerived1AsBase(self, current):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        raise umd1

    def unknownMostDerived1AsKnownIntermediate(self, current):
        umd1 = Test.UnknownMostDerived1()
        umd1.b = "UnknownMostDerived1.b"
        umd1.ki = "UnknownMostDerived1.ki"
        umd1.umd1 = "UnknownMostDerived1.umd1"
        raise umd1

    def unknownMostDerived2AsBase(self, current):
        umd2 = Test.UnknownMostDerived2()
        umd2.b = "UnknownMostDerived2.b"
        umd2.ui = "UnknownMostDerived2.ui"
        umd2.umd2 = "UnknownMostDerived2.umd2"
        raise umd2


class Server(TestHelper):
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
