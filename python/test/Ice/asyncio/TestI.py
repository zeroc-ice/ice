#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import asyncio
import Ice
import Test


# The implementation of the coroutines below assume the use of an Ice dispatcher which
# dispatch the calls on an asyncio event loop

class TestIntfI(Test.TestIntf):

    async def op(self, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that returning the result from the coroutine without await works
        return 5

    async def throwUserException1(self, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that raising the exception from the coroutine without await works
        raise Test.TestException()

    async def throwUserException2(self, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that raising the exception after from the coroutine after the await works
        await asyncio.sleep(0.01)
        raise Test.TestException()

    async def throwUnhandledException1(self, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that raising an unhandled exception from the coroutine without await works
        raise Exception("unexpected")

    async def throwUnhandledException2(self, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that raising an unhandled exception from the coroutine after the await works
        await asyncio.sleep(0.01)
        raise Exception("unexpected")

    async def sleep(self, ms, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that awaiting before returning the result works
        await asyncio.sleep(ms / 1000.0)

    async def callOpOn(self, proxy, current):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        # ensure that awaiting proxy invocations works
        await Ice.wrap_future(proxy.sleepAsync(10))
        return await Ice.wrap_future(proxy.opAsync())

    def shutdown(self, current=None):
        # make sure this is called from an asyncio event loop
        asyncio.get_running_loop()

        current.adapter.getCommunicator().shutdown()
