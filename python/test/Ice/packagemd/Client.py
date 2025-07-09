#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import AllTests
from TestHelper import TestHelper


class Client(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
