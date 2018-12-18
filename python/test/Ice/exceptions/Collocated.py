#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI
import AllTests


class Collocated(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.MessageSizeMax", "10")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0")
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.ThrowerI(), Ice.stringToIdentity("thrower"))
            # adapter.activate() // Don't activate OA to ensure collocation is used.
            AllTests.allTests(self, communicator)
