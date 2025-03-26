# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod

class EventLoopAdapter(ABC):
    """
    An adapter that allows applications to execute asynchronous code in a custom event loop.
    """

    @abstractmethod
    def runCoroutine(self, coroutine):
        """
        Run a coroutine in the application configured event loop. The Ice run time will call this method to run
        coroutines returned by async dispatch methods. This method is called from the Ice dispatch thread.

        Parameters
        ----------
        coroutine : coroutine
            The coroutine to run.

        Returns
        -------
        Future-like
            A Future-like object that can be used to wait for the completion of the coroutine.
        """
        pass

    @abstractmethod
    def wrapFuture(self, future):
        """
        Wraps an Ice.Future so that it can be awaited in the application event loop. The Ice run time calls this method
        before returning a future to the application.

        Parameters
        ----------
        future : Ice.Future
            The future to wrap.

        Returns
        -------
        Future-like
            A Future-like object that can be awaited in the application event loop.
        """
        pass
