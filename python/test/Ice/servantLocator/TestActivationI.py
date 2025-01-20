# Copyright (c) ZeroC, Inc.

import Test
import TestI


class TestActivationI(Test.TestActivation):
    def activateServantLocator(self, activate, current):
        if activate:
            current.adapter.addServantLocator(TestI.ServantLocatorI(""), "")
            current.adapter.addServantLocator(
                TestI.ServantLocatorI("category"), "category"
            )
        else:
            locator = current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = current.adapter.removeServantLocator("category")
            locator.deactivate("category")
