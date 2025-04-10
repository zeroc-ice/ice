# Copyright (c) ZeroC, Inc.

import sys
import asyncio
import Ice
import Ice.asyncio
import Test


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


async def allTestsAsync(helper, communicator):
    p = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint(num=0)}")

    sys.stdout.write("testing invocation... ")
    sys.stdout.flush()
    test(await p.opAsync() == 5)
    await p.sleepAsync(0)
    await p.sleepAsync(20)
    test(await p.callOpOnAsync(p) == 5)
    print("ok")

    sys.stdout.write("testing builtin in operations... ")
    sys.stdout.flush()
    await p.ice_pingAsync()
    test(await p.ice_isAAsync("::Test::TestIntf"))
    test(await p.ice_idsAsync() == ["::Ice::Object", "::Test::TestIntf"])
    test(await p.ice_idAsync() == "::Test::TestIntf")
    p1 = await Test.TestIntfPrx.checkedCastAsync(p)
    test(p1 is not None)
    print("ok")

    sys.stdout.write("testing exceptions... ")
    sys.stdout.flush()
    try:
        await p.throwUserException1Async()
        test(False)
    except Test.TestException:
        pass
    try:
        await p.throwUserException2Async()
        test(False)
    except Test.TestException:
        pass
    try:
        await p.throwUnhandledException1Async()
        test(False)
    except Ice.UnknownException:
        pass
    try:
        await p.throwUnhandledException2Async()
        test(False)
    except Ice.UnknownException:
        pass
    print("ok")

    sys.stdout.write("testing cancellation... ")
    sys.stdout.flush()

    future = p.sleepAsync(500)
    future.cancel()
    try:
        await future
        test(False)
    except asyncio.CancelledError:
        test(future.cancelled() and future.cancelled())

    future = p.sleepAsync(500)
    future.cancel()
    try:
        await future
        test(False)
    except asyncio.CancelledError:
        # Wait a little to ensure the cancellation propagates to the Ice future
        await asyncio.sleep(0.01)
        test(future.cancelled() and future.cancelled())

    # Try to cancel a done future
    future = p.opAsync()
    while not future.done():
        await asyncio.sleep(0.01)
    future.cancel()

    print("ok")

    sys.stdout.write("testing communicator shutdownCompleted... ")
    sys.stdout.flush()

    testCommunicator = Ice.initialize(eventLoop=asyncio.get_running_loop())
    shutdownCompletedFuture = testCommunicator.shutdownCompleted()
    test(not shutdownCompletedFuture.done())
    test(not testCommunicator.isShutdown())

    destroyFuture = testCommunicator.destroyAsync()

    await shutdownCompletedFuture

    test(shutdownCompletedFuture.done())
    test(testCommunicator.isShutdown())

    await destroyFuture
    print("ok")

    sys.stdout.write("testing communicator asynchronous context manager... ")
    sys.stdout.flush()

    initData = Ice.InitializationData()
    # Copy the existing properties to the new communicator for the SSL setup
    initData.properties = communicator.getProperties()
    initData.eventLoopAdapter = Ice.asyncio.EventLoopAdapter(asyncio.get_running_loop())

    async with Ice.initialize(initData=initData) as testCommunicator:
        p1 = Test.TestIntfPrx(testCommunicator, f"test:{helper.getTestEndpoint(num=0)}")
        await p1.opAsync()

    try:
        await p1.opAsync()
        test(False)
    except Ice.CommunicatorDestroyedException:
        pass

    async with Ice.initialize(initData=initData) as testCommunicator:
        p1 = Test.TestIntfPrx(testCommunicator, f"test:{helper.getTestEndpoint(num=0)}")
        await p1.opAsync()

        # Destroy the communicator, this is uncommon but possible. It will force the C++ destroyAsync callback
        # used in the asynchronous context manager implementation to be called from the event loop thread.
        testCommunicator.destroy()

        try:
            await p1.opAsync()
            test(False)
        except Ice.CommunicatorDestroyedException:
            pass

    print("ok")

    # Shutdown the server
    await p.shutdownAsync()
