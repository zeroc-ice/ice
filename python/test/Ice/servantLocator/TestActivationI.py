# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time
import Ice, Test, TestI


class TestActivationI(Test.TestActivation):

    def activateServantLocator(self, activate, current=None):
        if activate:
            current.adapter.addServantLocator(TestI.ServantLocatorI(""), "")
            current.adapter.addServantLocator(TestI.ServantLocatorI("category"), "category")
        else:
            locator = current.adapter.removeServantLocator("")
            locator.deactivate("")
            locator = current.adapter.removeServantLocator("category")
            locator.deactivate("category")
