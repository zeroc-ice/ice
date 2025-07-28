#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("-I. Test.ice Forward.ice ServerPrivate.ice")

import TestI

import Ice


class Server(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = TestI.InitialI(adapter)
            adapter.add(initial, Ice.stringToIdentity("initial"))
            adapter.add(TestI.F2I(), Ice.stringToIdentity("F21"))
            uoet = TestI.UnexpectedObjectExceptionTestI()
            adapter.add(uoet, Ice.stringToIdentity("uoet"))
            adapter.activate()
            communicator.waitForShutdown()
