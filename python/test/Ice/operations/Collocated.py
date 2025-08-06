#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import TestI

import AllTests
import Ice


class Collocated(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.BatchAutoFlushSize", "100")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            prx = adapter.add(TestI.MyDerivedClassI(), Ice.stringToIdentity("test"))
            # adapter.activate() // Don't activate OA to ensure collocation is used.
            if prx.ice_getConnection():
                raise RuntimeError("collocation doesn't work")
            AllTests.allTests(self, communicator)
