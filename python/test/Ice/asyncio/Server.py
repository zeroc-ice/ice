#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
import asyncio
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI


class Server(TestHelper):

    def run(self, args):

        async def runAsync():
            loop = asyncio.get_event_loop()
            initData = Ice.InitializationData()
            initData.properties = self.createTestProperties(args)
            initData.properties.setProperty("Ice.Warn.Dispatch", "0")
            initData.dispatcher = lambda method, connection: loop.call_soon_threadsafe(method)
            with self.initialize(initData) as communicator:
                communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
                adapter = communicator.createObjectAdapter("TestAdapter")
                adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
                adapter.activate()
                await loop.run_in_executor(None, communicator.waitForShutdown)

        asyncio.run(runAsync(), debug=True)
