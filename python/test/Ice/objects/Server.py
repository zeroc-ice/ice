#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice ServerPrivate.ice")
import TestI
import Ice


def MyValueFactory(type):
    if type == '::Test::I':
        return TestI.II()
    elif type == '::Test::J':
        return TestI.JI()
    elif type == '::Test::H':
        return TestI.HI()
    assert(False)  # Should never be reached


class Server(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getValueFactoryManager().add(MyValueFactory, '::Test::I')
            communicator.getValueFactoryManager().add(MyValueFactory, '::Test::J')
            communicator.getValueFactoryManager().add(MyValueFactory, '::Test::H')
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = TestI.InitialI(adapter)
            adapter.add(initial, Ice.stringToIdentity("initial"))
            uoet = TestI.UnexpectedObjectExceptionTestI()
            adapter.add(uoet, Ice.stringToIdentity("uoet"))
            adapter.activate()
            communicator.waitForShutdown()
