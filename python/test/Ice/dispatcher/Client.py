#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice
import Dispatcher
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
        d = Dispatcher.Dispatcher()
        initData.dispatcher = d.dispatch

        with self.initialize(initData=initData) as communicator:
            AllTests.allTests(self, communicator)

        Dispatcher.Dispatcher.terminate()
