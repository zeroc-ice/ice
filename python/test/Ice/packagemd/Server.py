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
TestHelper.loadSlice('--all -I. Test.ice')
import Test
import Test1
import testpkg
import modpkg
import Ice


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


class Server(TestHelper):

    def run(self, args):

        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(InitialI(), Ice.stringToIdentity("initial"))
            adapter.activate()
            communicator.waitForShutdown()
