#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import asyncio
import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import TestI

import Ice


class Server(TestHelper):
    def run(self, args):
        async def runAsync():
            initData = Ice.InitializationData()
            initData.properties = self.createTestProperties(args)
            initData.properties.setProperty("Ice.Warn.Dispatch", "0")
            initData.eventLoopAdapter = Ice.asyncio.EventLoopAdapter(asyncio.get_running_loop())

            async with self.initialize(initData) as communicator:
                communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
                adapter = communicator.createObjectAdapter("TestAdapter")
                adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
                adapter.activate()
                await communicator.shutdownCompleted()

        asyncio.run(runAsync(), debug=True)
