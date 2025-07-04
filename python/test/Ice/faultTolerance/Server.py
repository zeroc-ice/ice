#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import os
import Ice
from TestHelper import TestHelper
import Test


class TestI(Test.TestIntf):
    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

    def abort(self, current):
        print("aborting...")
        os._exit(0)

    def idempotentAbort(self, current):
        os._exit(0)

    def pid(self, current):
        return os.getpid()


class Server(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        port = 0
        for arg in args:
            if arg[0] == "-":
                continue

            if port > 0:
                raise RuntimeError("only one port can be specified")

            port = int(arg)

        if port <= 0:
            raise RuntimeError("no port specified\n")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", self.getTestEndpoint(num=port)
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
