# Copyright (c) ZeroC, Inc.

import logging
import traceback

from .Future import Future
from .LocalExceptions import TimeoutException
from .LocalExceptions import InvocationCanceledException

class InvocationFuture(Future):
    def __init__(self, operation, asyncInvocationContext):
        Future.__init__(self)
        assert asyncInvocationContext
        self._operation = operation
        self._asyncInvocationContext = asyncInvocationContext
        self._sent = False
        self._sentSynchronously = False
        self._sentCallbacks = []

    def cancel(self):
        self._asyncInvocationContext.cancel()
        return Future.cancel(self)

    def add_done_callback_async(self, fn):
        def callback():
            try:
                fn(self)
            except Exception:
                self._warn("done callback raised exception")

        with self._condition:
            if self._state == Future.StateRunning:
                self._doneCallbacks.append(fn)
                return
        self._asyncInvocationContext.callLater(callback)

    def is_sent(self):
        with self._condition:
            return self._sent

    def is_sent_synchronously(self):
        with self._condition:
            return self._sentSynchronously

    def add_sent_callback(self, fn):
        with self._condition:
            if not self._sent:
                self._sentCallbacks.append(fn)
                return
        fn(self, self._sentSynchronously)

    def add_sent_callback_async(self, fn):
        def callback():
            try:
                fn(self, self._sentSynchronously)
            except Exception:
                self._warn("sent callback raised exception")

        with self._condition:
            if not self._sent:
                self._sentCallbacks.append(fn)
                return
        self._asyncInvocationContext.callLater(callback)

    def sent(self, timeout=None):
        with self._condition:
            if not self._wait(timeout, lambda: not self._sent):
                raise TimeoutException()

            if self._state == Future.StateCancelled:
                raise InvocationCanceledException()
            elif self._exception:
                raise self._exception
            else:
                return self._sentSynchronously

    def set_sent(self, sentSynchronously):
        callbacks = []
        with self._condition:
            if self._sent:
                return

            self._sent = True
            self._sentSynchronously = sentSynchronously
            callbacks = self._sentCallbacks
            self._sentCallbacks = []
            self._condition.notify_all()

        for callback in callbacks:
            try:
                callback(self, sentSynchronously)
            except Exception:
                self._warn("sent callback raised exception")

    def operation(self):
        return self._operation

    def _warn(self, msg):
        communicator = self.communicator()
        if communicator:
            if (communicator.getProperties().getIcePropertyAsInt("Ice.Warn.AMICallback") > 0):
                communicator.getLogger().warning(
                    "Ice.Future: " + msg + ":\n" + traceback.format_exc()
                )
        else:
            logging.getLogger("Ice.Future").exception(msg)

__all__ = ["InvocationFuture"]
