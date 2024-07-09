# Copyright (c) ZeroC, Inc. All rights reserved.

import Ice.OperationMode_ice

class Current(object):
    """
        Information about the current method invocation for servers. Each operation on the server has a
        Current as its implicit final parameter. Current is mostly used for Ice services. Most
        applications ignore this parameter.
    Members:
    adapter --  The object adapter.
    con --  Information about the connection over which the current method invocation was received. If the invocation is
        direct due to collocation optimization, this value is set to null.
    id --  The Ice object identity.
    facet --  The facet.
    operation --  The operation name.
    mode --  The mode of the operation.
    ctx --  The request context, as received from the client.
    requestId --  The request id unless oneway (0).
    encoding --  The encoding version used to encode the input and output parameters.
    """

    def __init__(
        self,
        adapter=None,
        con=None,
        id=None,
        facet="",
        operation="",
        mode=Ice.OperationMode.Normal,
        ctx=None,
        requestId=0,
        encoding=None,
    ):
        self.adapter = adapter
        self.con = con
        self.id = Ice.Identity() if id is None else id
        self.facet = facet
        self.operation = operation
        self.mode = mode
        self.ctx = ctx
        self.requestId = requestId
        self.encoding = Ice.EncodingVersion() if encoding is None else encoding
