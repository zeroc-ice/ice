# Copyright (c) ZeroC, Inc.

import Test
import TestAMDI


class TestActivationAMDI(Test.TestActivation):
    def activateServantLocator(self, activate, current):
        if activate:
            current.adapter.addServantLocator(TestAMDI.ServantLocatorI(""), "")
            current.adapter.addServantLocator(
                TestAMDI.ServantLocatorI("category"), "category"
            )
        else:
            locator = current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = current.adapter.removeServantLocator("category")
            locator.deactivate("category")
