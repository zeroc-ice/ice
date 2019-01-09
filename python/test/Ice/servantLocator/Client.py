#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            obj = AllTests.allTests(self, communicator)
            obj.shutdown()
