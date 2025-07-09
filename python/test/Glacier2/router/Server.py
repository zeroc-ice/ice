#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import Test

import Ice
from TestHelper import TestHelper


class CallbackI(Test.Callback):
    def initiateCallback(self, proxy, current):
        proxy.callback(current.ctx)

    def initiateCallbackEx(self, proxy, current):
        proxy.callbackEx(current.ctx)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", self.getTestEndpoint(num=0))
            adapter = communicator.createObjectAdapter("CallbackAdapter")

            # The test allows "c1" as category.
            adapter.add(CallbackI(), Ice.stringToIdentity("c1/callback"))

            # The test allows "c2" as category.
            adapter.add(CallbackI(), Ice.stringToIdentity("c2/callback"))

            # The test rejects "c3" as category.
            adapter.add(CallbackI(), Ice.stringToIdentity("c3/callback"))

            # The test allows the prefixed userid.
            adapter.add(CallbackI(), Ice.stringToIdentity("_userid/callback"))
            adapter.activate()
            communicator.waitForShutdown()
