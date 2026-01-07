# Copyright (c) ZeroC, Inc.

from typing import final

import IcePy


@final
class ImplicitContext:
    """
    Represents the request context associated with a communicator.
    When you make a remote invocation without an explicit request context parameter, Ice uses the per-proxy request
    context (if any) combined with the ``ImplicitContext`` associated with your communicator.

    The property ``Ice.ImplicitContext`` controls if your communicator has an associated implicit context,
    and when it does, whether this implicit context is per-thread or shared by all threads:

    - None (default):
        No implicit context at all.
    - PerThread:
        The implementation maintains a context per thread.
    - Shared:
        The implementation maintains a single context shared by all threads.
    """

    def __init__(self, impl: IcePy.ImplicitContext):
        self._impl = impl

    def getContext(self) -> dict[str, str]:
        """
        Gets a copy of the request context maintained by this object.

        Returns
        -------
        dict[str, str]
            A copy of the request context.
        """
        return self._impl.getContext()

    def setContext(self, newContext: dict[str, str]):
        """
        Sets the request context.

        Parameters
        ----------
        newContext : dict[str, str]
            The new request context.
        """
        self._impl.setContext(newContext)

    def containsKey(self, key: str) -> bool:
        """
        Checks if the specified key has an associated value in the request context.

        Parameters
        ----------
        key : str
            The key.

        Returns
        -------
        bool
            ``True`` if the key has an associated value, ``False`` otherwise.
        """
        return self._impl.containsKey(key)

    def get(self, key: str) -> str:
        """
        Gets the value associated with the specified key in the request context.

        Parameters
        ----------
        key : str
            The key.

        Returns
        -------
        str
            The value associated with the key, or the empty string if no value is associated with the key.
            :func:`containsKey` allows you to distinguish between an empty-string value and no value at all.
        """
        return self._impl.get(key)

    def put(self, key: str, value: str) -> str | None:
        """
        Creates or updates a key/value entry in the request context.

        Parameters
        ----------
        key : str
            The key.
        value : str
            The value.

        Returns
        -------
        str | None
            The previous value associated with the key, if any.
        """
        return self._impl.put(key, value)

    def remove(self, key: str) -> str | None:
        """
        Removes the entry for the specified key in the request context.

        Parameters
        ----------
        key : str
            The key.

        Returns
        -------
        str | None
            The value associated with the key, if any.
        """
        return self._impl.remove(key)


__all__ = ["ImplicitContext"]
