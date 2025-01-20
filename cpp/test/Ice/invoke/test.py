# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, Server, TestSuite


TestSuite(
    __file__,
    [
        ClientServerTestCase(),
        ClientServerTestCase(
            name="client/server array", servers=[Server(args=["--array"])]
        ),
        ClientServerTestCase(
            name="client/server async", servers=[Server(args=["--async"])]
        ),
    ],
)
