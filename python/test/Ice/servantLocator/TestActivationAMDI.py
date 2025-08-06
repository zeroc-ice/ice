# Copyright (c) ZeroC, Inc.

import TestAMDI
from generated.test.Ice.servantLocator import Test

import Ice


class TestActivationAMDI(Test.TestActivation):
    def activateServantLocator(self, activate, current: Ice.Current):
        if activate:
            current.adapter.addServantLocator(TestAMDI.ServantLocatorI(""), "")
            current.adapter.addServantLocator(TestAMDI.ServantLocatorI("category"), "category")
        else:
            locator = current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = current.adapter.removeServantLocator("category")
            locator.deactivate("category")
