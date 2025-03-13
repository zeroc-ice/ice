# Copyright (c) ZeroC, Inc.

import sys
import asyncio
import threading
import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


async def allTestsAsync(helper, communicator):
    p = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint(num=0)}")

    sys.stdout.write("testing invocation... ")
    sys.stdout.flush()
    test(await Ice.wrap_future(p.opAsync()) == 5)
    await Ice.wrap_future(p.sleepAsync(0))
    await Ice.wrap_future(p.sleepAsync(20))
    test(await Ice.wrap_future(p.callOpOnAsync(p)) == 5)
    print("ok")

    sys.stdout.write("testing exceptions... ")
    sys.stdout.flush()
    try:
        await Ice.wrap_future(p.throwUserException1Async())
        test(False)
    except Test.TestException:
        pass
    try:
        await Ice.wrap_future(p.throwUserException2Async())
        test(False)
    except Test.TestException:
        pass
    try:
        await Ice.wrap_future(p.throwUnhandledException1Async())
        test(False)
    except Ice.UnknownException:
        pass
    try:
        await Ice.wrap_future(p.throwUnhandledException2Async())
        test(False)
    except Ice.UnknownException:
        pass
    print("ok")

    sys.stdout.write("testing cancellation... ")
    sys.stdout.flush()

    future = p.sleepAsync(500)
    asyncioFuture = Ice.wrap_future(future)
    future.cancel()
    try:
        await asyncioFuture
        test(False)
    except asyncio.CancelledError:
        test(future.cancelled() and asyncioFuture.cancelled())

    future = p.sleepAsync(500)
    asyncioFuture = Ice.wrap_future(future)
    asyncioFuture.cancel()
    try:
        await asyncioFuture
        test(False)
    except asyncio.CancelledError:
        # Wait a little to ensure the cancellation propagates to the Ice future
        await asyncio.sleep(0.01)
        test(future.cancelled() and asyncioFuture.cancelled())

    # Try to cancel a done future
    future = p.opAsync()
    while not future.done():
        await asyncio.sleep(0.01)
    Ice.wrap_future(future).cancel()

    print("ok")

    await Ice.wrap_future(p.shutdownAsync())

    sys.stdout.write("testing communicator shutdownCompleted... ")
    sys.stdout.flush()

    testCommunicator = Ice.initialize()
    shutdownCompletedFuture = Ice.wrap_future(testCommunicator.shutdownCompleted())
    test(not shutdownCompletedFuture.done())
    test(not testCommunicator.isShutdown())

    # Call destroy() from a separate thread to avoid blocking the event loop
    def destroy_communicator():
        testCommunicator.destroy()

    destroy_thread = threading.Thread(target=destroy_communicator)
    destroy_thread.start()

    await shutdownCompletedFuture

    test(shutdownCompletedFuture.done())
    test(testCommunicator.isShutdown())

    destroy_thread.join()

    print("ok")
