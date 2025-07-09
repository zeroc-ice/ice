#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import TestI

import AllTests
import Ice
from TestHelper import TestHelper


class Client(TestHelper):
    def run(self, args):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)
        initData.sliceLoader = TestI.customSliceLoader
        with self.initialize(initData=initData) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
