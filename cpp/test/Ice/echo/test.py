# Copyright (c) ZeroC, Inc.


from __future__ import annotations

from Util import ClientServerTestCase, Driver, Server, TestSuite


class EchoServerTestCase(ClientServerTestCase):
    def __init__(self):
        ClientServerTestCase.__init__(self, "server", server=Server(quiet=True, waitForShutdown=False))

    def runClientSide(self, current: Driver.Current) -> None:
        pass


TestSuite(__name__, [EchoServerTestCase()])
