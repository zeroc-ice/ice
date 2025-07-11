#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

import AllTests


class Client(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            proxy = AllTests.allTests(self, communicator)
            proxy.shutdown()
