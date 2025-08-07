# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import asyncio
from collections.abc import Awaitable, Coroutine
from typing import TYPE_CHECKING

from ..EventLoopAdapter import EventLoopAdapter as Ice_EventLoopAdapter
from ..Future import wrap_future

if TYPE_CHECKING:
    import Ice


class EventLoopAdapter(Ice_EventLoopAdapter):
    """
    An asyncio implementation of the Ice.EventLoopAdapter interface.

    This event loop adapter is automatically used by the Ice run time when an asyncio event loop is used to initialize
    the communicator.
    """

    def __init__(self, eventLoop: asyncio.AbstractEventLoop):
        self._eventLoop = eventLoop

    def runCoroutine(self, coroutine: Coroutine) -> Ice.FutureLike:
        return asyncio.run_coroutine_threadsafe(coroutine, self._eventLoop)

    def wrapFuture(self, future: Ice.Future) -> Awaitable:
        return wrap_future(future, loop=self._eventLoop)
