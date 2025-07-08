# Copyright (c) ZeroC, Inc.

import Ice
import asyncio


class EventLoopAdapter(Ice.EventLoopAdapter):
    """
    An asyncio implementation of the Ice.EventLoopAdapter interface.

    This event loop adapter is automatically used by the Ice run time when an asyncio event loop is used to initialize
    the communicator.
    """

    def __init__(self, eventLoop):
        self._eventLoop = eventLoop

    def runCoroutine(self, coroutine):
        return asyncio.run_coroutine_threadsafe(coroutine, self._eventLoop)

    def wrapFuture(self, future):
        return Ice.wrap_future(future, loop=self._eventLoop)
