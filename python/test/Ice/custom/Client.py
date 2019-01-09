#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")

try:
    TestHelper.loadSlice("TestNumPy.ice")
except ImportError:
    pass

import AllTests


class Client(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            custom = AllTests.allTests(self, communicator)
            custom.shutdown()
