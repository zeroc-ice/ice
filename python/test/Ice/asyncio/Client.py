#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import asyncio

from TestHelper import TestHelper

import AllTests
import Ice


class Client(TestHelper):
    def run(self, args):
        async def runAsync():
            initData = Ice.InitializationData()
            initData.properties = self.createTestProperties(args)
            initData.eventLoopAdapter = Ice.asyncio.EventLoopAdapter(asyncio.get_running_loop())

            async with self.initialize(initData) as communicator:
                await AllTests.allTestsAsync(self, communicator)

        asyncio.run(runAsync(), debug=True)
