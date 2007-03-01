#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Latency.ice')
import Demo

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("Latency")
        adapter.add(Demo.Ping(), self.communicator().stringToIdentity("ping"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
