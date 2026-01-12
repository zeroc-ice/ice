# Copyright (c) ZeroC, Inc.

import logging
from collections.abc import Callable

import IcePy

from .Future import Future
from .LocalExceptions import InvocationCanceledException, TimeoutException


class InvocationFuture(Future):
    """
    A Future object representing the result of an AMI (Asynchronous Method Invocation) request.

    Instances of ``InvocationFuture`` are returned by AMI requests, and applications can either wait for the result
    of the request to become available, or register a callback which will be invoked when the result becomes available.

    Notes
    -----
    This class provides the same functionality as :class:`Ice.Future` with the addition of "sent callbacks";
    callbacks which are invoked when the request has been sent.
    """

    def __init__(self, asyncInvocationContext: IcePy.AsyncInvocationContext):
        Future.__init__(self)
        assert asyncInvocationContext
        self._asyncInvocationContext = asyncInvocationContext
        self._sent = False
        self._sentSynchronously = False
        self._sentCallbacks = []

    def cancel(self) -> bool:
        """
        Cancels the invocation.

        This function invokes :func:`Future.cancel` to cancel the underlying future.
        If the cancellation is successful, the associated invocation is also cancelled.

        Cancelling an invocation prevents a queued invocation from being sent.
        If the invocation has already been sent, cancellation ensures that any reply from the server is ignored.

        After cancellation, :func:`done` returns ``True``, and attempting to retrieve the result raises an
        :class:`Ice.InvocationCanceledException`.

        Notes
        -----
        Cancellation is a local operation with no effect on the server.

        Returns
        -------
        bool
            ``True`` if the operation was successfully cancelled, ``False`` otherwise.
        """
        cancelled = Future.cancel(self)
        if cancelled:
            self._asyncInvocationContext.cancel()
        return cancelled

    def is_sent(self) -> bool:
        """
        Checks if the request has been sent.

        Returns
        -------
        bool
            ``True`` if the request has been sent, otherwise ``False``.
        """
        with self._condition:
            return self._sent

    def is_sent_synchronously(self) -> bool:
        """
        Checks if the request was sent synchronously.

        Returns
        -------
        bool
            ``True`` if the request was sent synchronously, otherwise ``False``.
        """
        with self._condition:
            return self._sentSynchronously

    def add_sent_callback(self, fn: Callable[[bool], None]) -> None:
        """
        Attaches a callback function which will be called when the invocation is sent.
        If the invocation has already been sent, ``fn`` is called immediately from the calling thread.

        Parameters
        ----------
        fn : Callable[[bool], None]
            The function to execute when the invocation is sent.
            It accepts a single boolean argument which is only ``True`` if the invocation was sent synchronously.
        """
        with self._condition:
            if not self._sent:
                self._sentCallbacks.append(fn)
                return
        fn(self._sentSynchronously)

    def sent(self, timeout: int | float | None = None) -> bool:
        """
        Waits until the invocation has been sent.

        If the invocation has not been sent, this function will wait up to ``timeout``-many seconds for it to send.

        Parameters
        ----------
        timeout : int | float | None, optional
            Maximum time (in seconds) to wait for the invocation to be sent.
            If ``None`` (the default), this function waits indefinitely.

        Returns
        -------
        bool
            ``True`` if the invocation was sent synchronously, otherwise ``False``.

        Raises
        ------
        TimeoutException
            If the invocation was not sent within the specified timeout.
        InvocationCanceledException
            If the invocation was cancelled before it was sent.
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

    def set_sent(self, sentSynchronously: bool):
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
                callback(sentSynchronously)
            except Exception as ex:
                logging.getLogger("Ice.Future").exception("sent callback raised exception: %s", ex)


__all__ = ["InvocationFuture"]
