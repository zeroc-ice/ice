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
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.MyClassI(), Ice.stringToIdentity("test"))
            adapter.addFacet(TestI.MyClassI(), Ice.stringToIdentity("test"), "foo")
            # Don't activate OA to ensure collocation is used.
            # adapter.activate()
            AllTests.allTests(self, communicator, True)
