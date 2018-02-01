#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
Ice.loadSlice('--all -I. Test.ice')
import Test, Test1, testpkg, modpkg

class InitialI(Test.Initial):

    def getTest1C2AsObject(self, current):
        return Test1.C2()

    def getTest1C2AsC1(self, current):
        return Test1.C2()

    def getTest1C2AsC2(self, current):
        return Test1.C2()

    def throwTest1E2AsE1(self, current):
        raise Test1.E2()

    def throwTest1E2AsE2(self, current):
        raise Test1.E2()

    def throwTest1Def(self, current):
        raise Test1._def()

    def getTest2C2AsObject(self, current):
        return testpkg.Test2.C2()

    def getTest2C2AsC1(self, current):
        return testpkg.Test2.C2()

    def getTest2C2AsC2(self, current):
        return testpkg.Test2.C2()

    def throwTest2E2AsE1(self, current):
        raise testpkg.Test2.E2()

    def throwTest2E2AsE2(self, current):
        raise testpkg.Test2.E2()

    def getTest3C2AsObject(self, current):
        return modpkg.Test3.C2()

    def getTest3C2AsC1(self, current):
        return modpkg.Test3.C2()

    def getTest3C2AsC2(self, current):
        return modpkg.Test3.C2()

    def throwTest3E2AsE1(self, current):
        raise modpkg.Test3.E2()

    def throwTest3E2AsE2(self, current):
        raise modpkg.Test3.E2()

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    initial = InitialI()
    adapter.add(initial, Ice.stringToIdentity("initial"))
    adapter.activate()

    communicator.waitForShutdown()
    return True

try:
    with Ice.initialize(sys.argv) as communicator:
         status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
