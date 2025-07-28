# Copyright (c) ZeroC, Inc.


import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import TestI


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            locator = TestI.ServantLocatorI()
            adapter.addServantLocator(locator, "")
            adapter.activate()
            adapter.waitForDeactivate()
