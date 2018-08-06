#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty('Ice.ThreadPool.Client.Size', '2')
        properties.setProperty('Ice.ThreadPool.Client.SizeWarn', '0')
        properties.setProperty("Ice.BatchAutoFlushSize", "100")

        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator)
            # Test multiple destroy calls
            communicator.destroy()
            communicator.destroy()
