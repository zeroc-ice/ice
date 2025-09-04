#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Callback.ice"])

from generated.test.Glacier2.router import Test

import Ice


class CallbackI(Test.Callback):
    @override
    def initiateCallback(self, proxy: Test.CallbackReceiverPrx | None, current: Ice.Current) -> None:
        assert proxy is not None
        proxy.callback(current.ctx)

    @override
    def initiateCallbackEx(self, proxy: Test.CallbackReceiverPrx | None, current: Ice.Current) -> None:
        assert proxy is not None
        proxy.callbackEx(current.ctx)

    @override
    def shutdown(self, current: Ice.Current) -> None:
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args: list[str]) -> None:
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
