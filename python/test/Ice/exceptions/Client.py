#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

import AllTests


class Client(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.MessageSizeMax", "10")
        properties.setProperty("Ice.Warn.Connections", "0")
        with self.initialize(properties=properties) as communicator:
            thrower = AllTests.allTests(self, communicator)
            thrower.shutdown()
