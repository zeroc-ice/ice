#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import TestI

import Ice
from TestHelper import TestHelper


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints",
                "{0}:{1}".format(self.getTestEndpoint(), self.getTestEndpoint(protocol="udp")),
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
