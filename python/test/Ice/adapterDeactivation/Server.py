# Copyright (c) ZeroC, Inc.

import TestI

from TestHelper import TestHelper


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            locator = TestI.ServantLocatorI()
            adapter.addServantLocator(locator, "")
            adapter.activate()
            adapter.waitForDeactivate()
