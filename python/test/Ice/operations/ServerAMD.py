#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

#
# We want to test coroutines but older versions of Python cannot
# load a source file that uses the async/await keywords, so we
# have two versions of MyDerivedClassI.
#
from TestAMDCoroI import MyDerivedClassI

from TestHelper import TestHelper

TestHelper.loadSlice("Test.ice")
import Ice


class ServerAMD(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we supress
        # this warning.
        #
        properties.setProperty("Ice.Warn.Dispatch", "0")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", self.getTestEndpoint()
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
