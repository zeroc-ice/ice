# Copyright (c) ZeroC, Inc.

import asyncio
import concurrent.futures
import logging
import threading
import time
from abc import ABC, abstractmethod
from collections.abc import Awaitable, Callable, Generator
from typing import Any, Generic, Protocol, Self, TypeAlias, TypeVar, overload

from .LocalExceptions import InvocationCanceledException, TimeoutException

# Type variable for the result type of the Future
_T = TypeVar("_T")
_T_co = TypeVar("_T_co", covariant=True)


class Future(Awaitable[_T]):
    """
    A Future object representing the result of an asynchronous operation.
    """

    def __init__(self):
        self._result: _T | None = None
        self._exception: BaseException | None = None
        self._condition = threading.Condition()
        self._doneCallbacks = []
        self._state = Future.StateRunning

    def __await__(self) -> Generator[Any, None, _T]:
        if not self.done():
            yield self
        return self.result()

    def cancel(self):
        """
        Attempt to cancel the operation.

        If the operation is already running or has completed, it cannot be cancelled, and the method returns False.
        Otherwise, the operation is cancelled, and the method returns True.

        Returns
        -------
        bool
            True if the operation was cancelled, False otherwise.
        """
        callbacks = []
        with self._condition:
            if self._state == Future.StateDone:
                return False

            if self._state == Future.StateCancelled:
                return True

            self._state = Future.StateCancelled
            callbacks = self._doneCallbacks
            self._doneCallbacks = []
            self._condition.notify_all()

        self._callCallbacks(callbacks)

        return True

    def cancelled(self):
        """
        Check if the future has been cancelled.

        Returns
        -------
        bool
            True if the future was cancelled using `cancel()`, otherwise False.
        """
        with self._condition:
            return self._state == Future.StateCancelled

    def running(self):
        """
        Check if the future is still running.

        Returns
        -------
        bool
            True if the operation is currently executing and cannot be cancelled,
            otherwise False.
        """
        with self._condition:
            return self._state == Future.StateRunning

    def done(self):
        """
        Check if the future has completed or been cancelled.

        Returns
        -------
        bool
            True if the operation has completed (successfully or with an exception)
            or has been cancelled, otherwise False.
        """
        with self._condition:
            return self._state in [Future.StateCancelled, Future.StateDone]

    def add_done_callback(self, fn: Callable) -> None:
        """
        Attach a callback to be executed when the future completes.

        The callback `fn` is called with the future as its only argument once the future completes or is cancelled.
        Registered callbacks are executed in the order they were added.

        If the future is already complete, `fn` is called immediately from the calling thread.

        Parameters
        ----------
        fn : callable
            The function to execute upon completion. Takes the future as its argument.
        """
        with self._condition:
            if self._state == Future.StateRunning:
                self._doneCallbacks.append(fn)
                return
        fn(self)

    def result(self, timeout: int | float | None = None) -> _T:
        """
        Retrieve the result of the future.

        If the operation has not completed, this method waits up to `timeout` seconds for it to finish. If the
        timeout is reached, a `TimeoutException` is raised.

        If the future was cancelled before completing, an `InvocationCanceledException` is raised.

        If the operation raised an exception, this method raises the same exception.

        Parameters
        ----------
        timeout : int or float, optional
            Maximum time (in seconds) to wait for the result. If `None`, the method waits indefinitely until the
            operation completes.

        Returns
        -------
        object
            The result of the operation.

        Raises
        ------
        TimeoutException
            If the operation has not completed within the specified timeout.
        InvocationCanceledException
            If the operation was cancelled before completing.
        Exception
            If the operation raised an exception.
        """
        with self._condition:
            if not self._wait(timeout, lambda: self._state == Future.StateRunning):
                raise TimeoutException()

            if self._state == Future.StateCancelled:
                raise InvocationCanceledException()
            elif self._exception:
                raise self._exception
            else:
                # We can't check if _result is None here, because it is valid to have a result of None.
                # i.e. Future[None]
                return self._result  # type: ignore[return-value]

    def exception(self, timeout: int | float | None = None) -> BaseException | None:
        """
        Retrieve the exception raised by the operation.

        If the operation has not completed, this method waits up to `timeout` seconds for it to finish. If the timeout
        is reached, a `TimeoutException` is raised.

        If the future was cancelled before completing, an `InvocationCanceledException` is raised.

        If the operation completed successfully without raising an exception, `None` is returned.

        Parameters
        ----------
        timeout : int or float, optional
            Maximum time (in seconds) to wait for the exception. If `None`, the method waits indefinitely until the
            operation completes.

        Returns
        -------
        BaseException or None
            The exception raised by the operation, or `None` if the operation completed successfully.

        Raises
        ------
        TimeoutException
            If the operation has not completed within the specified timeout.
        InvocationCanceledException
            If the operation was cancelled before completing.
        """
        with self._condition:
            if not self._wait(timeout, lambda: self._state == Future.StateRunning):
                raise TimeoutException()
            if self._state == Future.StateCancelled:
                raise InvocationCanceledException()
            else:
                return self._exception

    def set_result(self, result: _T):
        """
        Set the result of the future and mark it as completed.

        This method stores the provided `result` and transitions the future's state to "done". Any registered
        callbacks are executed after the state update.

        If the future is not in a running state, this method has no effect.

        Parameters
        ----------
        result : object
            The result value to store in the future.
        """
        callbacks = []
        with self._condition:
            if self._state != Future.StateRunning:
                return
            self._result = result
            self._state = Future.StateDone
            callbacks = self._doneCallbacks
            self._doneCallbacks = []
            self._condition.notify_all()

        self._callCallbacks(callbacks)

    def set_exception(self, ex: BaseException):
        """
        Set an exception for the future and mark it as completed.

        This method stores the provided exception `ex` and transitions the future's state to "done". Any registered
        callbacks are executed after the state update.

        If the future is not in a running state, this method has no effect.

        Parameters
        ----------
        ex : BaseException
            The exception to store in the future.
        """
        callbacks = []
        with self._condition:
            if self._state != Future.StateRunning:
                return
            self._exception = ex
            self._state = Future.StateDone
            callbacks = self._doneCallbacks
            self._doneCallbacks = []
            self._condition.notify_all()
        self._callCallbacks(callbacks)

    @staticmethod
    def completed(result: _T) -> "Future[_T]":
        f = Future()
        f.set_result(result)
        return f

    def _wait(self, timeout: float | None, testFn: Callable[[], bool]) -> bool:
        # Must be called with _condition acquired

        while testFn():
            if timeout:
                start = time.time()
                self._condition.wait(timeout)
                # Subtract the elapsed time so far from the timeout
                timeout -= time.time() - start
                if timeout <= 0:
                    return False
            else:
                self._condition.wait()

        return True

    def _callCallbacks(self, callbacks: list[Callable[[Self], None]]):
        for callback in callbacks:
            try:
                callback(self)
            except Exception:
                self._warn("done callback raised exception")

    def _warn(self, msg: str):
        # TODO why are we not using the Ice logger?
        logging.getLogger("Ice.Future").exception(msg)

    StateRunning = "running"
    StateCancelled = "cancelled"
    StateDone = "done"


def wrap_future(future: Future | asyncio.Future, *, loop: asyncio.AbstractEventLoop | None = None) -> asyncio.Future:
    """
    Wrap an :class:`Ice.Future` object into an ``asyncio.Future``.

    This function converts an Ice.Future into an asyncio.Future to allow integration of Ice's
    asynchronous operations with Python's asyncio framework. If the provided future is already
    an asyncio.Future, it is returned unchanged.

    If the Ice.Future is already completed, the asyncio.Future is immediately resolved. Otherwise,
    completion callbacks are registered to ensure that the asyncio.Future reflects the state of the
    Ice.Future, including result propagation, exception handling, and cancellation.

    Parameters
    ----------
    future : Future or asyncio.Future
        The Ice.Future object to wrap. If an asyncio.Future is passed, it is returned as-is.

    loop : asyncio.AbstractEventLoop, optional
        The event loop to associate with the asyncio.Future. If not provided, the current event loop
        is used.

    Returns
    -------
    asyncio.Future
        A future that mirrors the state of the input Ice.Future.

    Raises
    ------
    AssertionError
        If `future` is not an instance of Ice.Future.
    """

    if isinstance(future, asyncio.Future):
        return future

    assert isinstance(future, Future), "Ice.Future is expected, got {!r}".format(future)

    @overload
    def forwardCompletion(sourceFuture: Future, targetFuture: asyncio.Future): ...

    @overload
    def forwardCompletion(sourceFuture: asyncio.Future, targetFuture: Future): ...

    def forwardCompletion(sourceFuture: Future | asyncio.Future, targetFuture: Future | asyncio.Future):
        if not targetFuture.done():
            if sourceFuture.cancelled():
                targetFuture.cancel()
            elif (sourceException := sourceFuture.exception()) is not None:
                targetFuture.set_exception(sourceException)
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


class FutureLike(Protocol[_T_co]):
    """A protocol that defines the interface for objects that behave like a Future."""

    def add_done_callback(self, callback: Callable[["FutureLike"], Any], /) -> None:
        """Add a callback to be run when the Future is done.

        Args:
            callback: A callable that takes the Future object as its only argument.
                    Will be called when the Future completes (successfully,
                    with exception, or cancelled).

        Note: Using positional-only parameter (/) to match both asyncio and
            concurrent.futures implementations regardless of parameter name.
        """
        ...

    def result(self, timeout: int | float | None = None) -> _T_co:
        """Return the result of the Future."""
        ...


__all__ = ["Future", "wrap_future", "FutureLike"]
