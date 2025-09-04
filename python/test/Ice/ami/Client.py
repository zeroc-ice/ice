#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

import AllTests


class Client(TestHelper):
    def run(self, args: list[str]):
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
