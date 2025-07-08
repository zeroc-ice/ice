# Copyright (c) ZeroC, Inc.

import asyncio
import Test


# The implementation of the coroutines below assume the use of an Ice executor which
# dispatch the calls on an asyncio event loop


class TestIntfI(Test.TestIntf):
    async def op(self, current):
        await asyncio.sleep(0)
        # ensure that returning the result from the coroutine without await works
        return 5

    async def throwUserException1(self, current):
        await asyncio.sleep(0)
        # ensure that raising the exception from the coroutine without await works
        raise Test.TestException()

    async def throwUserException2(self, current):
        await asyncio.sleep(0)
        # ensure that raising the exception after from the coroutine after the await works
        await asyncio.sleep(0.01)
        raise Test.TestException()

    async def throwUnhandledException1(self, current):
        await asyncio.sleep(0)

        # ensure that raising an unhandled exception from the coroutine without await works
        raise Exception("unexpected")

    async def throwUnhandledException2(self, current):
        # ensure that raising an unhandled exception from the coroutine after the await works
        await asyncio.sleep(0.01)
        raise Exception("unexpected")

    async def sleep(self, ms, current):
        # ensure that awaiting before returning the result works
        await asyncio.sleep(ms / 1000.0)

    async def callOpOn(self, proxy, current):
        # ensure that awaiting proxy invocations works
        await proxy.sleepAsync(10)
        return await proxy.opAsync()

    async def shutdown(self, current):
        await asyncio.sleep(0)

        current.adapter.getCommunicator().shutdown()
