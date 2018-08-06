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
import TestI
import Ice


class Collocated(TestHelper):

    def run(self, args):

        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.AMICallback", "0")
        #
        # This test kills connections, so we don't want warnings.
        #
        properties.setProperty("Ice.Warn.Connections", "0")

        with self.initialize(properties=properties) as communicator:

            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", "default -p 12011")
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter2 = communicator.createObjectAdapter("ControllerAdapter")

            testController = TestI.TestIntfControllerI(adapter)

            adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
            adapter.add(TestI.TestIntfII(), Ice.stringToIdentity("test2"))
            # adapter.activate() # Collocated test doesn't need to active the OA

            adapter2.add(testController, Ice.stringToIdentity("testController"))
            # adapter2.activate() # Collocated test doesn't need to active the OA

            AllTests.allTests(self, communicator, True)
