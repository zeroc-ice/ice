#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

#
# This file should only be used in Python >= 3.5.
#

import asyncio


#
# This class defines an __await__ method so that coroutines can call 'await <future>'.
#
# Python 2.x rejects this code with a syntax error because a return statement is not allowed in a generator.
#
class FutureBase(object):
    def __await__(self):
        if not self.done():
            yield self
        return self.result()


def wrap_future(future, *, loop=None):
    '''Wrap Ice.Future object into an asyncio.Future.'''
    if isinstance(future, asyncio.Future):
        return future

    assert isinstance(future, FutureBase), 'Ice.Future is expected, got {!r}'.format(future)

    def forwardCompletion(sourceFuture, targetFuture):
        if not targetFuture.done():
            if sourceFuture.cancelled():
                targetFuture.cancel()
            elif sourceFuture.exception():
                targetFuture.set_exception(sourceFuture.exception())
            else:
                targetFuture.set_result(sourceFuture.result())

    if loop is None:
        loop = asyncio.get_event_loop()
    asyncioFuture = loop.create_future()

    if future.done():
        # As long as no done callbacks are registered, completing the asyncio future should be thread safe
        # even if the future is constructed with a loop which isn't the current thread's loop.
        forwardCompletion(future, asyncioFuture)
    else:
        asyncioFuture.add_done_callback(lambda f: forwardCompletion(asyncioFuture, future))
        future.add_done_callback(lambda f: loop.call_soon_threadsafe(forwardCompletion, future, asyncioFuture))

    return asyncioFuture
