# Copyright (c) ZeroC, Inc. All rights reserved.

class BatchRequestInterceptor(object):
    """Base class for batch request interceptor. A subclass must define the enqueue method."""

    def enqueue(self, request, queueCount, queueSize):
        """Invoked when a request is batched."""
        pass
