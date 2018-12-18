# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class EchoServerTestCase(ClientServerTestCase):

    def __init__(self):
        ClientServerTestCase.__init__(self, "server", server=Server(quiet=True, waitForShutdown=False))

    def runClientSide(self, current):
        pass

TestSuite(__name__, [EchoServerTestCase()])
