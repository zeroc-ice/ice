#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper
import AllTests
import Ice


class Client(TestHelper):
    def run(self, args):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)
        initData.sliceLoader = AllTests.customSliceLoader
        with self.initialize(initData=initData) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
