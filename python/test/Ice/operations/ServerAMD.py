#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

# TODO: This is no longer relevant. All supported Python versions provide async/await keywords.
# We want to test coroutines but older versions of Python cannot
# load a source file that uses the async/await keywords, so we
# have two versions of MyDerivedClassI.
#
from TestAMDCoroI import MyDerivedClassI

from TestHelper import TestHelper
import Ice


class ServerAMD(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we suppress
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
