#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import TestActivationAMDI
import TestAMDI
from TestHelper import TestHelper

import Ice


class ServerAMD(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("Ice.Warn.Dispatch", "0")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.addServantLocator(TestAMDI.ServantLocatorI("category"), "category")
            adapter.addServantLocator(TestAMDI.ServantLocatorI(""), "")
            adapter.add(TestAMDI.TestI(), Ice.stringToIdentity("asm"))
            adapter.add(
                TestActivationAMDI.TestActivationAMDI(),
                Ice.stringToIdentity("test/activation"),
            )

            adapter.activate()
            adapter.waitForDeactivate()
