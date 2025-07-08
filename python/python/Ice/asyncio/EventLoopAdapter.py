# Copyright (c) ZeroC, Inc.

import Ice
import asyncio
from collections.abc import Awaitable, Coroutine
from ..Future import FutureLike
from asyncio import AbstractEventLoop


class EventLoopAdapter(Ice.EventLoopAdapter):
    """
    An asyncio implementation of the Ice.EventLoopAdapter interface.

    This event loop adapter is automatically used by the Ice run time when an asyncio event loop is used to initialize
    the communicator.
    """

    def __init__(self, eventLoop: AbstractEventLoop):
        self._eventLoop = eventLoop

    def runCoroutine(self, coroutine: Coroutine) -> FutureLike:
        # Convert the concurrent.futures.Future to an awaitable by wrapping it with asyncio.wrap_future
        return asyncio.run_coroutine_threadsafe(coroutine, self._eventLoop)

    def wrapFuture(self, future: Ice.Future) -> Awaitable:
        return Ice.wrap_future(future, loop=self._eventLoop)
