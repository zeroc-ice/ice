# Copyright (c) ZeroC, Inc.

from __future__ import annotations

from abc import ABC, abstractmethod
from collections.abc import Awaitable, Coroutine
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .Future import Future, FutureLike


class EventLoopAdapter(ABC):
    """
    An adapter that allows applications to execute asynchronous code in a custom event loop.
    """

    @abstractmethod
    def runCoroutine(self, coroutine: Coroutine) -> FutureLike:
        """
        Runs a coroutine in the application-configured event loop. The Ice run time will call this function to run
        coroutines returned by async dispatch methods. This function is called from the Ice dispatch thread.

        Parameters
        ----------
        coroutine : Coroutine
            The coroutine to run.

        Returns
        -------
        FutureLike
            A Future-like object that can be used to wait for the completion of the coroutine.
        """
        pass

    @abstractmethod
    def wrapFuture(self, future: Future) -> Awaitable:
        """
        Wraps an :class:`Ice.Future` so that it can be awaited in the application event loop.
        The Ice run time calls this function before returning a future to the application.

        Parameters
        ----------
        future : Ice.Future
            The future to wrap.

        Returns
        -------
        Awaitable
            An awaitable object that can be awaited in the application event loop.
        """
        pass
