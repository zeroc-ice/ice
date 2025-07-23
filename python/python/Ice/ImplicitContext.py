# Copyright (c) ZeroC, Inc.

from typing import final

import IcePy


@final
class ImplicitContext:
    """
    An interface to associate implicit contexts with communicators.

    When you make a remote invocation without an explicit context parameter, Ice uses the per-proxy context (if any)
    combined with the ImplicitContext associated with the communicator.

    Ice provides several implementations of ImplicitContext. The implementation used depends on the value of the
    `Ice.ImplicitContext` property.

    None (default)
        No implicit context at all.
    PerThread
        The implementation maintains a context per thread.
    Shared
        The implementation maintains a single context shared by all threads.


    ImplicitContext also provides a number of operations to create, update, or retrieve an entry in the underlying
    context without first retrieving a copy of the entire context.
    """

    def __init__(self, impl: IcePy.ImplicitContext):
        self._impl = impl

    def getContext(self) -> dict[str, str]:
        """
        Get a copy of the underlying context.

        Returns
        -------
        dict
            A copy of the underlying context.
        """
        return self._impl.getContext()

    def setContext(self, newContext: dict[str, str]):
        """
        Set the underlying context.

        Parameters
        ----------
        newContext : dict
            The new context to set.
        """
        self._impl.setContext(newContext)

    def containsKey(self, key: str) -> bool:
        """
        Check if this key has an associated value in the underlying context.

        Parameters
        ----------
        key : str
            The key to check.

        Returns
        -------
        bool
            True if the key has an associated value, False otherwise.
        """
        return self._impl.containsKey(key)

    def get(self, key: str) -> str:
        """
        Get the value associated with the given key in the underlying context.

        Returns an empty string if no value is associated with the key. Use `containsKey` to distinguish between an
        empty-string value and no value at all.

        Parameters
        ----------
        key : str
            The key to retrieve the value for.

        Returns
        -------
        str
            The value associated with the key, or an empty string if no value is associated with the key.
        """
        return self._impl.get(key)

    def put(self, key: str, value: str) -> str | None:
        """
        Create or update a key/value entry in the underlying context.

        Parameters
        ----------
        key : str
            The key to create or update.
        value : str
            The value to associate with the key.

        Returns
        -------
        str | None
            The previous value associated with the key, if any, otherwise None.
        """
        return self._impl.put(key, value)

    def remove(self, key: str) -> str | None:
        """
        Remove the entry for the given key in the underlying context.

        Parameters
        ----------
        key : str
            The key to remove.

        Returns
        -------
        str | None
            The value associated with the key, if any, otherwise None.
        """
        return self._impl.remove(key)


__all__ = ["ImplicitContext"]
