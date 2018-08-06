#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice('Callback.ice')
import Ice
import Test


class CallbackI(Test.Callback):

    def initiateCallback(self, proxy, current):
        proxy.callback(current.ctx)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("DeactivatedAdapter.Endpoints", "default -p 12011")
            communicator.createObjectAdapter("DeactivatedAdapter")

            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", "default -p 12010")
            adapter = communicator.createObjectAdapter("CallbackAdapter")
            adapter.add(CallbackI(), Ice.stringToIdentity("callback"))
            adapter.activate()
            communicator.waitForShutdown()
