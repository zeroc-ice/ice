#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

import TestI

import Ice


class Server(TestHelper):
    def run(self, args: list[str]) -> None:
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.RemoteCommunicatorFactoryI(), Ice.stringToIdentity("factory"))
            adapter.activate()

            communicator.waitForShutdown()
