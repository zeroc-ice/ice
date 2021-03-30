#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty('Ice.Warn.Connections', '0')
        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator)
