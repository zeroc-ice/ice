#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

TestHelper.loadSlice("Test.ice Forward.ice ClientPrivate.ice")
import AllTests
import Ice
import TestI


class Client(TestHelper):
    def run(self, args):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)
        initData.sliceLoader = TestI.customSliceLoader
        with self.initialize(initData=initData) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
