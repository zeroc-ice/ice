#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

import TestI

import AllTests
import Ice


class Collocated(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0")
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.ThrowerI(), Ice.stringToIdentity("thrower"))
            # adapter.activate() // Don't activate OA to ensure collocation is used.
            AllTests.allTests(self, communicator)
