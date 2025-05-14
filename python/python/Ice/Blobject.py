# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod
from .Object import Object

class Blobject(Object, ABC):
    """
    Special-purpose servant base class that allows a subclass to handle synchronous Ice invocations as "blobs" of
    bytes.

    This class serves as a base for creating servants that can process encoded Ice operation arguments and return
    results or exceptions in an encoded form. Subclasses must implement the `ice_invoke` method to handle the
    invocation logic.
    """

    @abstractmethod
    def ice_invoke(self, bytes, current):
        """
        Dispatches a synchronous Ice invocation.

        The operation's arguments are encoded in the `bytes` parameter. The return value must be a tuple of two
        values: the first is a boolean indicating whether the operation succeeded (True) or raised a user exception
        (False), and the second is the encoded form of the operation's results or the user exception.

        Parameters
        ----------
        bytes : bytes
            The encoded operation arguments.
        current : Ice.Current
            The current invocation context.

        Returns
        -------
        tuple of (bool, bytes)
            A tuple containing:
                - bool True if the operation succeeded, False if it raised a user exception.
                - bytes The encoded form of the operation's results or the user exception.
        """
        pass
