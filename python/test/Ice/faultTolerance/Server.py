#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import os
import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Test


class TestI(Test.TestIntf):

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def abort(self, current=None):
        print("aborting...")
        os._exit(0)

    def idempotentAbort(self, current=None):
        os._exit(0)

    def pid(self, current=None):
        return os.getpid()


class Server(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        #
        # In this test, we need a longer server idle time, otherwise
        # our test servers may time out before they are used in the
        # test.
        #
        properties.setProperty("Ice.ServerIdleTime", "120")  # Two minutes.

        port = 0
        for arg in args:

            if arg[0] == '-':
                continue

            if port > 0:
                raise RuntimeError("only one port can be specified")

            port = int(arg)

        if port <= 0:
            raise RuntimeError("no port specified\n")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint(num=port))
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
