# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, Client, EchoServer, TestSuite


TestSuite(
    __file__,
    [
        ClientServerTestCase(name="client/echo server", clients=[Client()], servers=[EchoServer()]),
    ],
)
