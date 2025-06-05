# Copyright (c) ZeroC, Inc.

import logging
import traceback

from .Future import Future
from .LocalExceptions import TimeoutException
from .LocalExceptions import InvocationCanceledException

class InvocationFuture(Future):
    """
    A Future object representing the result of an AMI (Asynchronous Method Invocation) request.

    InvocationFuture objects are returned by AMI requests. The application can use an InvocationFuture object to
    wait for the result of the AMI request or register a callback that will be invoked when the result becomes
    available.

    This class provides the same functionality as `Future`, with the addition of "sent callbacks," which are invoked
    when the request is sent.
    """

    def __init__(self, operation, asyncInvocationContext):
        Future.__init__(self)
        assert asyncInvocationContext
        self._operation = operation
        self._asyncInvocationContext = asyncInvocationContext
        self._sent = False
        self._sentSynchronously = False
        self._sentCallbacks = []

    def cancel(self):
        """
        Cancels the invocation.

        This method invokes :py:meth:`Future.cancel` to cancel the underlying future. If the cancellation is
        successful, the associated invocation is also cancelled.

        Cancelling an invocation prevents a queued invocation from being sent. If the invocation has already been sent,
        cancellation ensures that any reply from the server is ignored.

        Cancellation is a local operation with no effect on the server.

        After cancellation, :py:meth:`done` returns ``True``, and attempting to retrieve the result raises an
        :py:exc:`Ice.InvocationCanceledException`.

        Returns
        -------
        bool
            ``True`` if the operation was successfully cancelled, ``False`` otherwise.
        """
        cancelled = Future.cancel(self)
        if cancelled:
            self._asyncInvocationContext.cancel()
        return cancelled

    def is_sent(self):
        """
        Check if the request has been sent.

        Returns
        -------
        bool
            True if the request has been sent, otherwise False.
        """
        with self._condition:
            return self._sent

    def is_sent_synchronously(self):
        """
        Check if the request was sent synchronously.

        Returns
        -------
        bool
            True if the request was sent synchronously, otherwise False.
        """
        with self._condition:
            return self._sentSynchronously

    def add_sent_callback(self, fn):
        """
        Attach a callback to be executed when the invocation is sent.

        The callback `fn` is called with the future as its first argument and a boolean as its
        second argument, indicating whether the invocation was sent synchronously.

        If the future has already been sent, `fn` is called immediately from the calling thread.

        Parameters
        ----------
        fn : callable
            The function to execute when the invocation is sent.
        """
        with self._condition:
            if not self._sent:
                self._sentCallbacks.append(fn)
                return
        fn(self, self._sentSynchronously)

    def sent(self, timeout=None):
        """
        Wait for the operation to be sent.

        This method waits up to `timeout` seconds for the operation to be sent and then returns
        whether it was sent synchronously.

        If the operation has not been sent within the specified time, a `TimeoutException` is raised.
        If the future was cancelled before being sent, an `InvocationCanceledException` is raised.

        Parameters
        ----------
        timeout : int or float, optional
            Maximum time (in seconds) to wait for the operation to be sent. If `None`, the method waits
            indefinitely.

        Returns
        -------
        bool
            True if the operation was sent synchronously, otherwise False.

        Raises
        ------
        TimeoutException
            If the operation was not sent within the specified timeout.
        InvocationCanceledException
            If the operation was cancelled before it was sent.
        """
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
