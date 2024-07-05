# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class BatchRequestInterceptor(object):
    """Base class for batch request interceptor. A subclass must
    define the enqueue method."""

    def __init__(self):
        pass

    def enqueue(self, request, queueCount, queueSize):
        """Invoked when a request is batched."""
        pass
