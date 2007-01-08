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

Ice.loadSlice('Callback.ice')
import Demo

class CallbackSenderI(Demo.CallbackSender):
    def initiateCallback(self, proxy, current=None):
        print "initiating callback"
        try:
            proxy.callback(current.ctx)
        except:
            traceback.print_exc()

    def shutdown(self, current=None):
        print "Shutting down..."
        try:
            current.adapter.getCommunicator().shutdown()
        except:
            traceback.print_exc()

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("Callback.Server")
        adapter.add(CallbackSenderI(), self.communicator().stringToIdentity("callback"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
