#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice Forward.ice ClientPrivate.ice")
import Ice
import TestI
import AllTests


class Collocated(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = TestI.InitialI(adapter)
            adapter.add(initial, Ice.stringToIdentity("initial"))
            adapter.add(TestI.F2I(), Ice.stringToIdentity("F21"))
            adapter.add(TestI.UnexpectedObjectExceptionTestI(), Ice.stringToIdentity("uoet"))
            # adapter.activate() // Don't activate OA to ensure collocation is used.
            AllTests.allTests(self, communicator)
            # We must call shutdown even in the collocated case for cyclic dependency cleanup
            initial.shutdown()
