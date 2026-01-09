# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod

from .Current import Current
from .Object import Object


class ServantLocator(ABC):
    """
    An application-provided class that an object adapter uses to locate servants.

    Notes
    -----
    For simple cases, you should consider using a default servant instead (see :func:`ObjectAdapter.addDefaultServant`).
    """

    @abstractmethod
    def locate(self, current: Current) -> tuple[Object | None, object | None]:
        """
        Asks this servant locator to find and return a servant.

        Notes
        -----
        The caller (the object adapter) does not insert the returned servant into its Active Servant Map.
        This must be done by the servant locator implementation, if this is desired.

        The implementation can raise any exception, including :class:`UserException`.
        The Ice runtime will marshal this exception in the response.

        If you call :func:`locate` from your own code, you must also call :func:`finished` when you have finished using
        the servant, provided that :func:`locate` did not return ``None``.

        Parameters
        ----------
        current : Current
            Information about the incoming request for which a servant is required.

        Returns
        -------
        tuple[Object | None, object | None]

            A tuple containing the following:
                - retval : Object or None
                    The located servant, or ``None`` if no suitable servant was found.
                - cookie : object | None
                    A "cookie" that will be passed to :func:`finished`.
        """
        pass

    @abstractmethod
    def finished(self, current: Current, servant: Object, cookie: object | None):
        """
        Notifies this servant locator that the dispatch on the servant returned by :func:`locate` is complete.
        The object adapter calls this function only when :func:`locate` didn't return ``None``.

        Notes
        -----

        The implementation can raise any exception, including :class:`UserException`.
        The Ice runtime will marshal this exception in the response. If both the dispatch and :func:`finished` raise
        an exception, the exception raised by :func:`finished` prevails and is marshaled back to the client.

        Parameters
        ----------
        current : Current
            Information about the incoming request for which a servant was located.
        servant : Object
            The servant that was returned by :func:`locate`.
        cookie : object | None
            The cookie that was returned by :func:`locate`.
        """
        pass

    @abstractmethod
    def deactivate(self, category: str):
        """
        Notifies this servant locator that the object adapter in which it's installed is being deactivated.

        Parameters
        ----------
        category : str
            The category with which this servant locator was registered.
        """
        pass


__all__ = ["ServantLocator"]
