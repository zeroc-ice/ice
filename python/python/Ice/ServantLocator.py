# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod

from .Current import Current
from .Object import Object


class ServantLocator(ABC):
    """
    A servant locator is called by an object adapter to locate a servant that is not found in its active servant map.
    """

    @abstractmethod
    def locate(self, current: Current) -> tuple[Object | None, object]:
        """
        Called before a request is dispatched if a servant cannot be found in the object adapter's active servant map.

        Note that the object adapter does not automatically insert the returned servant into its active servant map.
        This must be done by the servant locator implementation, if desired. The `locate` method can throw any user
        exception. If it does, that exception is marshaled back to the client. If the Slice definition for the corresponding
        operation includes that user exception, the client receives that user exception; otherwise, the client receives
        `UnknownUserException`.

        If `locate` throws any exception, the Ice runtime does not call `finished`. If you call `locate` from your own code,
        you must also call `finished` when you have finished using the servant, provided that `locate` returned a non-null
        servant.

        Parameters
        ----------
        current : Current
            Information about the current operation for which a servant is required.

        Returns
        -------
        tuple[Object | None, object]

            A tuple containing the following:
                - retval : Object or None
                    The located servant, or None if no suitable servant has been found.
                - cookie : object
                    A "cookie" that will be passed to `finished`.

        Raises
        ------
        UserException
            The implementation can raise a `UserException` and the runtime will marshal it as the result of the invocation.
        """
        pass

    @abstractmethod
    def finished(self, current: Current, servant: Object, cookie: object):
        """
        Called by the object adapter after a request has been made.

        This operation is only called if `locate` was called prior to the request and returned a non-null servant.
        This operation can be used for cleanup purposes after a request.

        The `finished` method can throw any user exception. If it does, that exception is marshaled back to the client.
        If the Slice definition for the corresponding operation includes that user exception, the client receives that user
        exception; otherwise, the client receives `UnknownUserException`. If both the operation and `finished` throw an
        exception, the exception thrown by `finished` is marshaled back to the client.

        Parameters
        ----------
        current : Current
            Information about the current operation call for which a servant was located by `locate`.
        servant : Object
            The servant that was returned by `locate`.
        cookie : object
            The cookie that was returned by `locate`.

        Raises
        ------
        UserException
            The implementation can raise a `UserException` and the runtime will marshal it as the result of the invocation.
        """
        pass

    @abstractmethod
    def deactivate(self, category: str):
        """
        Called when the object adapter in which this servant locator is installed is destroyed.

        Parameters
        ----------
        category : str
            Indicates for which category the servant locator is being deactivated.
        """
        pass


__all__ = ["ServantLocator"]
