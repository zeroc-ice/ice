#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import AllTests
from TestHelper import TestHelper


class Client(TestHelper):
    def run(self, args):
        AllTests.allTests()
