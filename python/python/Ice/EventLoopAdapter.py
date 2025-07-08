# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod
from collections.abc import Awaitable, Coroutine
from .Future import Future, FutureLike, Any


class EventLoopAdapter(ABC):
    """
    An adapter that allows applications to execute asynchronous code in a custom event loop.
    """

    @abstractmethod
    def runCoroutine(self, coroutine: Coroutine) -> FutureLike:
        """
        Run a coroutine in the application configured event loop. The Ice run time will call this method to run
        coroutines returned by async dispatch methods. This method is called from the Ice dispatch thread.

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
        Wraps an Ice.Future so that it can be awaited in the application event loop. The Ice run time calls this method
        before returning a future to the application.

        Parameters
        ----------
        future : Ice.Future
            The future to wrap.

        Returns
        -------
        FutureLike
            A Future-like object that can be awaited in the application event loop.
        """
        pass
