# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod

class BatchRequestInterceptor(ABC):
    """
    Base class for batch request interceptors.

    A subclass must define the `enqueue` method.
    """

    @abstractmethod
    def enqueue(self, request, queueCount, queueSize):
        """
        Invoked when a request is batched.

        Parameters
        ----------
        request : IcePy.BatchRequest
        queueCount : int
            The number of requests currently in the queue.
        queueSize : int
            The number of bytes consumed by the requests currently in the queue.
        """
        pass
