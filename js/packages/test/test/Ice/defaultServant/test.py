# Copyright (c) ZeroC, Inc.

from Util import Client, ClientServerTestCase, EchoServer, TestSuite

TestSuite(
    __file__,
    [
        ClientServerTestCase(name="client/echo server", clients=[Client()], servers=[EchoServer()]),
    ],
)
