#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import asyncio
import Ice
import Test


class TestIntfI(Test.TestIntf):

    async def op(self, current):
        return 5

    async def throwUserException1(self, current):
        raise Test.TestException()

    async def throwUserException2(self, current):
        await asyncio.sleep(0.01)
        raise Test.TestException()

    async def throwUnhandledException1(self, current):
        raise Exception("unexpected")

    async def throwUnhandledException2(self, current):
        await asyncio.sleep(0.01)
        raise Exception("unexpected")

    async def sleep(self, ms, current):
        # This will throw if this isn't called from the asyncio loop
        asyncio.get_running_loop()
        await asyncio.sleep(ms / 1000.0)

    async def callOpOn(self, proxy, current):
        await Ice.wrap_future(proxy.sleepAsync(10))
        return await Ice.wrap_future(proxy.opAsync())

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()
