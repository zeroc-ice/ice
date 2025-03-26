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
    test(await p.opAsync() == 5)
    await p.sleepAsync(0)
    await p.sleepAsync(20)
    test(await p.callOpOnAsync(p) == 5)
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

    await p.shutdownAsync()

    sys.stdout.write("testing communicator shutdownCompleted... ")
    sys.stdout.flush()

    testCommunicator = Ice.initialize(eventLoop=asyncio.get_running_loop())
    shutdownCompletedFuture = testCommunicator.shutdownCompleted()
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
