#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice ClientPrivate.ice")

import AllTests


class Client(TestHelper):
    def run(self, args: list[str]):
        with self.initialize(args=args) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
