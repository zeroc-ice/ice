#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import Ice
import Executor
from TestHelper import TestHelper

TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):
    def run(self, args):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)

        #
        # Limit the send buffer size, this test relies on the socket
        # send() blocking after sending a given amount of data.
        #
        initData.properties.setProperty("Ice.TCP.SndSize", "50000")
        d = Executor.Executor()
        initData.executor = d.execute

        with self.initialize(initData=initData) as communicator:
            AllTests.allTests(self, communicator)

        Executor.Executor.terminate()
