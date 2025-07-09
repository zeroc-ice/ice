#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import AllTests
from TestHelper import TestHelper


class Client(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            obj = AllTests.allTests(self, communicator)
            obj.shutdown()
