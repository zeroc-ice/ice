# Copyright (c) ZeroC, Inc.

from typing import final

@final
class Current:
    """
    Provides information about an incoming request being dispatched.
    """

    def __init__(self, adapter, con, id, facet, operation, mode, ctx, requestId, encoding):
        self._adapter = adapter
        self._con = con
        self._id = id
        self._facet = facet
        self._operation = operation
        self._mode = mode
        self._ctx = ctx
        self._requestId = requestId
        self._encoding = encoding

    @property
    def adapter(self):
        """
        Ice.ObjectAdapter: The object adapter that received the request.
        """
        return self._adapter

    @property
    def con(self):
        """
        Optional[Ice.Connection]: The connection that received the request. It's None when the invocation and dispatch are collocated.
        """
        return self._con

    @property
    def id(self):
        """
        Ice.Identity: The identity of the target Ice object.
        """
        return self._id

    @property
    def facet(self):
        """
        str: The facet of the target Ice object.
        """
        return self._facet

    @property
    def operation(self):
        """
        str: The name of the operation.
        """
        return self._operation

    @property
    def mode(self):
        """
        OperationMode: The operation mode (idempotent or not).
        """
        return self._mode

    @property
    def ctx(self):
        """
        dict: The request context.
        """
        return self._ctx

    @property
    def requestId(self):
        """
        int: The request ID. 0 means the request is a one-way request.
        """
        return self._requestId

    @property
    def encoding(self):
        """
        EncodingVersion: The encoding of the request payload.
        """
        return self._encoding
