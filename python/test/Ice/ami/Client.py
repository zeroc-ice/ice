#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

import AllTests


class Client(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.AMICallback", "0")
        properties.setProperty("Ice.Warn.Connections", "0")

        #
        # Limit the send buffer size, this test relies on the socket
        # send() blocking after sending a given amount of data.
        #
        properties.setProperty("Ice.TCP.SndSize", "50000")

        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator, False)
            AllTests.allTestsFuture(self, communicator, False)
