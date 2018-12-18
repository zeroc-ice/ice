#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Test
import TestI

class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.ctx = None

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def getContext(self, current):
        return Ice.Future.completed(self.ctx)

    def echo(self, obj, current):
        return Ice.Future.completed(obj)

    def ice_isA(self, s, current):
        self.ctx = current.ctx
        return Test.MyDerivedClass.ice_isA(self, s, current)


class ServerAMD(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Connections", "0")
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
