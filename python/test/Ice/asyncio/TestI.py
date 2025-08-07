# Copyright (c) ZeroC, Inc.

import asyncio
from typing import override

from generated.test.Ice.asyncio import Test

import Ice

# The implementation of the coroutines below assume the use of an Ice executor which
# dispatch the calls on an asyncio event loop


class TestIntfI(Test.TestIntf):
    @override
    async def op(self, current: Ice.Current):
        await asyncio.sleep(0)
        # ensure that returning the result from the coroutine without await works
        return 5

    @override
    async def throwUserException1(self, current: Ice.Current):
        await asyncio.sleep(0)
        # ensure that raising the exception from the coroutine without await works
        raise Test.TestException()

    @override
    async def throwUserException2(self, current: Ice.Current):
        await asyncio.sleep(0)
        # ensure that raising the exception after from the coroutine after the await works
        await asyncio.sleep(0.01)
        raise Test.TestException()

    @override
    async def throwUnhandledException1(self, current: Ice.Current):
        await asyncio.sleep(0)

        # ensure that raising an unhandled exception from the coroutine without await works
        raise Exception("unexpected")

    @override
    async def throwUnhandledException2(self, current: Ice.Current):
        # ensure that raising an unhandled exception from the coroutine after the await works
        await asyncio.sleep(0.01)
        raise Exception("unexpected")

    @override
    async def sleep(self, ms: int, current: Ice.Current):
        # ensure that awaiting before returning the result works
        await asyncio.sleep(ms / 1000.0)

    @override
    async def callOpOn(self, proxy: Test.TestIntfPrx | None, current: Ice.Current):
        assert proxy is not None
        # ensure that awaiting proxy invocations works
        await proxy.sleepAsync(10)
        return await proxy.opAsync()

    @override
    async def shutdown(self, current: Ice.Current):
        await asyncio.sleep(0)
        current.adapter.getCommunicator().shutdown()
