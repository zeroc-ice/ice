#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import TestI

import AllTests
from TestHelper import TestHelper


class Collocated(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            locator = TestI.ServantLocatorI()

            adapter.addServantLocator(locator, "")
            # adapter.activate() // Don't activate OA to ensure collocation is used.

            AllTests.allTests(self, communicator)

            adapter.waitForDeactivate()
