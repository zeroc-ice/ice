# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, TestSuite

TestSuite(
    __name__,
    [
        ClientServerTestCase(),
        ClientServerTestCase(
            "client/server with 1.0 encoding",
            props={"Ice.Default.EncodingVersion": "1.0"},
        ),
    ],
)
