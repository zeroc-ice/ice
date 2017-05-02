#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, Ice
Ice.loadSlice('Callback.ice')
import Test

class CallbackI(Test.Callback):

    def initiateCallback(self, proxy, current):
        proxy.callback(current.ctx)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):

    def run(self, args):
        print(args)
        self.communicator().getProperties().setProperty("DeactivatedAdapter.Endpoints", "default -p 12011")
        self.communicator().createObjectAdapter("DeactivatedAdapter")

        self.communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "default -p 12010")
        adapter = self.communicator().createObjectAdapter("CallbackAdapter")
        adapter.add(CallbackI(), Ice.stringToIdentity("callback"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
status = app.main(sys.argv)
sys.exit(status)
