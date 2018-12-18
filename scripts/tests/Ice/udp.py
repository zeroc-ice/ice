# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from Util import *

class UdpTestCase(ClientServerTestCase):

    def setupServerSide(self, current):
        if current.config.android or current.config.xamarin:
            self.servers = [Server(ready="McastTestAdapter")]
        else:
            self.servers = [Server(args=[i], ready="McastTestAdapter") for i in range(0, 5)]

    def setupClientSide(self, current):
        if current.config.android:
            self.clients = [Client()]
        else:
            self.clients = [Client(args=[5])]

#
# With UWP, we can't run the UDP tests with the C++ servers (used when SSL is enabled).
#
options=lambda current: { "protocol": ["tcp", "ws"] } if current.config.uwp else {}

TestSuite(__name__, [ UdpTestCase() ], multihost=False, options=options)
