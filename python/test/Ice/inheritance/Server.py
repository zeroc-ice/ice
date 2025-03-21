#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

TestHelper.loadSlice("Test.ice")
import TestI
import Ice


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", self.getTestEndpoint()
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.InitialI(adapter), Ice.stringToIdentity("initial"))
            adapter.activate()
            communicator.waitForShutdown()
