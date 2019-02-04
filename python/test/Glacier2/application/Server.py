#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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
            communicator.getProperties().setProperty("DeactivatedAdapter.Endpoints", self.getTestEndpoint(num=1))
            communicator.createObjectAdapter("DeactivatedAdapter")

            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("CallbackAdapter")
            adapter.add(CallbackI(), Ice.stringToIdentity("callback"))
            adapter.activate()
            communicator.waitForShutdown()
