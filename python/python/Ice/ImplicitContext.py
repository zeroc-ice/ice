# Copyright (c) ZeroC, Inc. All rights reserved.

class ImplicitContext(object):
    """
    An interface to associate implicit contexts with communicators. When you make a remote invocation without an
    explicit context parameter, Ice uses the per-proxy context (if any) combined with the ImplicitContext
    associated with the communicator.
    Ice provides several implementations of ImplicitContext. The implementation used depends on the value
    of the Ice.ImplicitContext property.

    None (default)
    No implicit context at all.
    PerThread
    The implementation maintains a context per thread.
    Shared
    The implementation maintains a single context shared by all threads.

    ImplicitContext also provides a number of operations to create, update or retrieve an entry in the
    underlying context without first retrieving a copy of the entire context.
    """

    def __init__(self):
        if type(self) is ImplicitContext:
            raise RuntimeError("Ice.ImplicitContext is an abstract class")

    def getContext(self):
        """
            Get a copy of the underlying context.
        Returns: A copy of the underlying context.
        """
        raise NotImplementedError("method 'getContext' not implemented")

    def setContext(self, newContext):
        """
            Set the underlying context.
        Arguments:
        newContext -- The new context.
        """
        raise NotImplementedError("method 'setContext' not implemented")

    def containsKey(self, key):
        """
            Check if this key has an associated value in the underlying context.
        Arguments:
        key -- The key.
        Returns: True if the key has an associated value, False otherwise.
        """
        raise NotImplementedError("method 'containsKey' not implemented")

    def get(self, key):
        """
            Get the value associated with the given key in the underlying context. Returns an empty string if no value is
            associated with the key. containsKey allows you to distinguish between an empty-string value and no
            value at all.
        Arguments:
        key -- The key.
        Returns: The value associated with the key.
        """
        raise NotImplementedError("method 'get' not implemented")

    def put(self, key, value):
        """
            Create or update a key/value entry in the underlying context.
        Arguments:
        key -- The key.
        value -- The value.
        Returns: The previous value associated with the key, if any.
        """
        raise NotImplementedError("method 'put' not implemented")

    def remove(self, key):
        """
            Remove the entry for the given key in the underlying context.
        Arguments:
        key -- The key.
        Returns: The value associated with the key, if any.
        """
        raise NotImplementedError("method 'remove' not implemented")
