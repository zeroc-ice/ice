# Copyright (c) ZeroC, Inc. All rights reserved.

class Current:
    """
    Provides information about an incoming request being dispatched.
    """

    def __init__(self):
        if type(self) is Current:
            raise RuntimeError("Ice.Current is an abstract class")

    @property
    def adapter(self):
        """
        Ice.ObjectAdapter: The object adapter that received the request.
        """
        pass

    @property
    def con(self):
        """
        Optional[Ice.Connection]: The connection that received the request. It's None when the invocation and dispatch are collocated.
        """
        pass

    @property
    def id(self):
        """
        Ice.Identity: The identity of the target Ice object.
        """
        pass

    @property
    def facet(self):
        """
        str: The facet of the target Ice object.
        """
        pass

    @property
    def operation(self):
        """
        str: The name of the operation.
        """
        pass

    @property
    def mode(self):
        """
        OperationMode: The operation mode (idempotent or not).
        """
        pass

    @property
    def ctx(self):
        """
        dict: The request context.
        """
        pass

    @property
    def requestId(self):
        """
        int: The request ID. 0 means the request is a one-way request.
        """
        pass

    @property
    def encoding(self):
        """
        EncodingVersion: The encoding of the request payload.
        """
        pass

    __module__ = "Ice"
