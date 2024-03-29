#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper

TestHelper.loadSlice("Test.ice Forward.ice ServerPrivate.ice")
import TestI
import Ice


def MyValueFactory(type):
    if type == "::Test::I":
        return TestI.II()
    elif type == "::Test::J":
        return TestI.JI()
    elif type == "::Test::H":
        return TestI.HI()
    assert False  # Should never be reached


class Server(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::I")
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::J")
            communicator.getValueFactoryManager().add(MyValueFactory, "::Test::H")
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", self.getTestEndpoint()
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = TestI.InitialI(adapter)
            adapter.add(initial, Ice.stringToIdentity("initial"))
            adapter.add(TestI.F2I(), Ice.stringToIdentity("F21"))
            uoet = TestI.UnexpectedObjectExceptionTestI()
            adapter.add(uoet, Ice.stringToIdentity("uoet"))
            adapter.activate()
            communicator.waitForShutdown()
