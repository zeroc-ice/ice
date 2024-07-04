# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
import Ice.Context_ice
import Ice.Identity_ice
import Ice.OperationMode_ice
import Ice.Version_ice

# Included module Ice
_M_Ice = Ice.openModule("Ice")

# Start of module Ice
__name__ = "Ice"

if "Current" not in _M_Ice.__dict__:
    _M_Ice.Current = Ice.createTempClass()

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
            id=Ice._struct_marker,
            facet="",
            operation="",
            mode=_M_Ice.OperationMode.Normal,
            ctx=None,
            requestId=0,
            encoding=Ice._struct_marker,
        ):
            self.adapter = adapter
            self.con = con
            if id is Ice._struct_marker:
                self.id = _M_Ice.Identity()
            else:
                self.id = id
            self.facet = facet
            self.operation = operation
            self.mode = mode
            self.ctx = ctx
            self.requestId = requestId
            if encoding is Ice._struct_marker:
                self.encoding = _M_Ice.EncodingVersion()
            else:
                self.encoding = encoding

    _M_Ice.Current = Current
