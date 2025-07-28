#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import AllTests


class Client(TestHelper):
    def run(self, args):
        #
        # In this test, we need at least two threads in the
        # client side thread pool for nested AMI.
        #
        properties = self.createTestProperties(args)

        #
        # For this test, we want to disable retries.
        #
        properties.setProperty("Ice.RetryIntervals", "-1")

        #
        # Limit the send buffer size, this test relies on the socket
        # send() blocking after sending a given amount of data.
        #
        properties.setProperty("Ice.TCP.SndSize", "50000")

        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator)
