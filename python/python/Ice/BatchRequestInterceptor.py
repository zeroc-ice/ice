# Copyright (c) ZeroC, Inc.

class BatchRequestInterceptor:
    """
    Base class for batch request interceptor.

    A subclass must define the `enqueue` method.
    """

    def enqueue(self, request, queueCount, queueSize):
        """
        Invoked when a request is batched.

        Parameters
        ----------
        request : BatchRequest
        queueCount : int
        queueSize : int
        """
        pass
