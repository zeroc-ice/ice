#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper
import Ice
import TestI
import AllTests


class Collocated(TestHelper):
    def run(self, args):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)
        initData.properties.setProperty("Ice.Warn.Dispatch", "0")
        initData.sliceLoader = TestI.customSliceLoader
        with self.initialize(initData=initData) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = TestI.InitialI(adapter)
            adapter.add(initial, Ice.stringToIdentity("initial"))
            adapter.add(TestI.F2I(), Ice.stringToIdentity("F21"))
            adapter.add(TestI.UnexpectedObjectExceptionTestI(), Ice.stringToIdentity("uoet"))
            # adapter.activate() // Don't activate OA to ensure collocation is used.
            AllTests.allTests(self, communicator)
            # We must call shutdown even in the collocated case for cyclic dependency cleanup
            initial._shutdown()
