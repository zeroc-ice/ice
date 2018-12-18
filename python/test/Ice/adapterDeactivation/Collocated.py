#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests
import TestI


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
