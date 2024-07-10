# Copyright (c) ZeroC, Inc. All rights reserved.

from .Object import Object

class BlobjectAsync(Object):
    """Special-purpose servant base class that allows a subclass to
    handle asynchronous Ice invocations as "blobs" of bytes."""

    def ice_invoke(self, bytes, current):
        """Dispatch an asynchronous Ice invocation. The operation's
        arguments are encoded in the bytes argument. The result must be
        a tuple of two values: the first is a boolean indicating whether the
        operation succeeded (True) or raised a user exception (False), and
        the second is the encoded form of the operation's results or the user
        exception. The subclass can either return the tuple directly (for
        synchronous completion) or return a future that is eventually
        completed with the tuple."""
        pass
