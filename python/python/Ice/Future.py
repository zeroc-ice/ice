# Copyright (c) ZeroC, Inc.

import threading
import time
import asyncio
import logging
from .LocalExceptions import TimeoutException
from .LocalExceptions import InvocationCanceledException

#
# This class defines an __await__ method so that coroutines can call 'await <future>'.
#
class FutureBase(object):
    def __await__(self):
        if not self.done():
            yield self
        return self.result()


def wrap_future(future, *, loop=None):
    """Wrap Ice.Future object into an asyncio.Future."""
    if isinstance(future, asyncio.Future):
        return future

    assert isinstance(future, FutureBase), "Ice.Future is expected, got {!r}".format(
        future
    )

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
        asyncioFuture.add_done_callback(
            lambda f: forwardCompletion(asyncioFuture, future)
        )
        future.add_done_callback(
            lambda f: loop.call_soon_threadsafe(
                forwardCompletion, future, asyncioFuture
            )
        )

    return asyncioFuture


class Future(FutureBase):
    def __init__(self):
        self._result = None
        self._exception = None
        self._condition = threading.Condition()
        self._doneCallbacks = []
        self._state = Future.StateRunning

    def cancel(self):
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
        with self._condition:
            return self._state == Future.StateCancelled

    def running(self):
        with self._condition:
            return self._state == Future.StateRunning

    def done(self):
        with self._condition:
            return self._state in [Future.StateCancelled, Future.StateDone]

    def add_done_callback(self, fn):
        with self._condition:
            if self._state == Future.StateRunning:
                self._doneCallbacks.append(fn)
                return
        fn(self)

    def result(self, timeout=None):
        with self._condition:
            if not self._wait(timeout, lambda: self._state == Future.StateRunning):
                raise TimeoutException()

            if self._state == Future.StateCancelled:
                raise InvocationCanceledException()
            elif self._exception:
                raise self._exception
            else:
                return self._result

    def exception(self, timeout=None):
        with self._condition:
            if not self._wait(timeout, lambda: self._state == Future.StateRunning):
                raise TimeoutException()
            if self._state == Future.StateCancelled:
                raise InvocationCanceledException()
            else:
                return self._exception

    def set_result(self, result):
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

    def set_exception(self, ex):
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
    def completed(result):
        f = Future()
        f.set_result(result)
        return f

    def _wait(self, timeout, testFn=None):
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

    def _callCallbacks(self, callbacks):
        for callback in callbacks:
            try:
                callback(self)
            except Exception:
                self._warn("done callback raised exception")

    def _warn(self, msg):
        # TODO why are we not using the Ice logger?
        logging.getLogger("Ice.Future").exception(msg)

    StateRunning = "running"
    StateCancelled = "cancelled"
    StateDone = "done"

__all__ = ["Future", "wrap_future", "FutureBase"]
