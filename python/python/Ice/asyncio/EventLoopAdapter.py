# Copyright (c) ZeroC, Inc.

import asyncio
from collections.abc import Awaitable, Coroutine

from ..EventLoopAdapter import EventLoopAdapter as Ice_EventLoopAdapter
from ..Future import Future, FutureLike, wrap_future


class EventLoopAdapter(Ice_EventLoopAdapter):
    """
    An asyncio implementation of the Ice.EventLoopAdapter interface.

    This event loop adapter is automatically used by the Ice run time when an asyncio event loop is used to initialize
    the communicator.
    """

    def __init__(self, eventLoop: asyncio.AbstractEventLoop):
        self._eventLoop = eventLoop

    def runCoroutine(self, coroutine: Coroutine) -> FutureLike:
        # Convert the concurrent.futures.Future to an awaitable by wrapping it with asyncio.wrap_future
        return asyncio.run_coroutine_threadsafe(coroutine, self._eventLoop)

    def wrapFuture(self, future: Future) -> Awaitable:
        return wrap_future(future, loop=self._eventLoop)
