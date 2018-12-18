#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice('Test.ice')
import Ice
import TestI
import AllTests
import TestActivationI


class Collocated(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0")

            adapter = communicator.createObjectAdapter("TestAdapter")
            # adapter.activate() // Don't activate OA to ensure collocation is used.
            adapter.addServantLocator(TestI.ServantLocatorI("category"), "category")
            adapter.addServantLocator(TestI.ServantLocatorI(""), "")
            adapter.add(TestI.TestI(), Ice.stringToIdentity("asm"))
            adapter.add(TestActivationI.TestActivationI(), Ice.stringToIdentity("test/activation"))

            AllTests.allTests(self, communicator)

            adapter.deactivate()
            adapter.waitForDeactivate()
