#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from TestAMDI import MyDerivedClassI

import Ice


class ServerAMD(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we suppress
        # this warning.
        #
        properties.setProperty("Ice.Warn.Dispatch", "0")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
