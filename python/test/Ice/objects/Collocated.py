#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["-I.", "Test.ice", "Forward.ice", "ClientPrivate.ice", "ServerPrivate.ice"])

import TestI
from Objects import customSliceLoader

import AllTests
import Ice


class Collocated(TestHelper):
    def run(self, args: list[str]):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)
        initData.properties.setProperty("Ice.Warn.Dispatch", "0")
        initData.sliceLoader = customSliceLoader
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
