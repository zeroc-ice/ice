#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper
import Ice
import TestI


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints",
                self.getTestEndpoint()
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.MyClassI(), Ice.stringToIdentity("test"))
            adapter.addFacet(TestI.MyClassI(), Ice.stringToIdentity("test"), "foo")
            adapter.activate()
            communicator.waitForShutdown()
