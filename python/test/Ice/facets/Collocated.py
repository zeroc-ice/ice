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
            adapter.add(TestI.DI(), Ice.stringToIdentity("d"))
            adapter.addFacet(TestI.DI(), Ice.stringToIdentity("d"), "facetABCD")
            adapter.addFacet(TestI.FI(), Ice.stringToIdentity("d"), "facetEF")
            adapter.addFacet(TestI.HI(communicator), Ice.stringToIdentity("d"), "facetGH")

            # adapter.activate() // Don't activate OA to ensure collocation is used.

            AllTests.allTests(self, communicator)
