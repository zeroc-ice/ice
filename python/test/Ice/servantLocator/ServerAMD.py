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
TestHelper.loadSlice('Test.ice')
import Ice
import TestAMDI
import TestActivationAMDI


class ServerAMD(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.addServantLocator(TestAMDI.ServantLocatorI("category"), "category")
            adapter.addServantLocator(TestAMDI.ServantLocatorI(""), "")
            adapter.add(TestAMDI.TestI(), Ice.stringToIdentity("asm"))
            adapter.add(TestActivationAMDI.TestActivationAMDI(), Ice.stringToIdentity("test/activation"))

            adapter.activate()
            adapter.waitForDeactivate()
