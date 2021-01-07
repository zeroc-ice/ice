#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import asyncio
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):

        async def runAsync():
            with self.initialize(properties=self.createTestProperties(args)) as communicator:
                await AllTests.allTestsAsync(self, communicator)

        asyncio.run(runAsync(), debug=True)
