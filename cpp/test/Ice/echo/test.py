# Copyright (c) ZeroC, Inc.


from Util import ClientServerTestCase, Server, TestSuite


class EchoServerTestCase(ClientServerTestCase):
    def __init__(self):
        ClientServerTestCase.__init__(self, "server", server=Server(quiet=True, waitForShutdown=False))

    def runClientSide(self, current):
        pass


TestSuite(__name__, [EchoServerTestCase()])
