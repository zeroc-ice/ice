#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os, sys, traceback, time
import Ice, Test, TestAMDI

class TestActivationAMDI(Test.TestActivation):

    def activateServantLocator(self, activate, current=None):
        if(activate):
            current.adapter.addServantLocator(TestAMDI.ServantLocatorI(""), "")
            current.adapter.addServantLocator(TestAMDI.ServantLocatorI("category"), "category")
        else:
            locator = current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = current.adapter.removeServantLocator("category")
            locator.deactivate("category")
