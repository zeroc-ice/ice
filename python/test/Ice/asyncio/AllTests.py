#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys
import asyncio
import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


async def allTestsAsync(helper, communicator):

    sref = "test:{0}".format(helper.getTestEndpoint(num=0))
    obj = communicator.stringToProxy(sref)
    test(obj)

    p = Test.TestIntfPrx.uncheckedCast(obj)

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
        await Ice.wrap_future(p.throwUserException1())
        test(False)
    except Test.TestException:
        pass
    try:
        await Ice.wrap_future(p.throwUserException2())
        test(False)
    except Test.TestException:
        pass
    try:
        await Ice.wrap_future(p.throwUnhandledException1())
        test(False)
    except Ice.UnknownException:
        pass
    try:
        await Ice.wrap_future(p.throwUnhandledException2())
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
