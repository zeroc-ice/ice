# Copyright (c) ZeroC, Inc.

import TestI
from generated.test.Ice.servantLocator import Test


class TestActivationI(Test.TestActivation):
    def activateServantLocator(self, activate, current: Ice.Current):
        if activate:
            current.adapter.addServantLocator(TestI.ServantLocatorI(""), "")
            current.adapter.addServantLocator(TestI.ServantLocatorI("category"), "category")
        else:
            locator = current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = current.adapter.removeServantLocator("category")
            locator.deactivate("category")
