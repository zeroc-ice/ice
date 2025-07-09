#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import TestI

import Ice
from TestHelper import TestHelper


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", "{0} -t 10000".format(self.getTestEndpoint())
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.RemoteCommunicatorFactoryI(), Ice.stringToIdentity("factory"))
            adapter.activate()
            communicator.waitForShutdown()
