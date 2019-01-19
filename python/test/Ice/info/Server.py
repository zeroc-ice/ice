#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Ice
import TestI


class Server(TestHelper):

    def run(self, args):

        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", "{0}:{1}".format(self.getTestEndpoint(),
                                                          self.getTestEndpoint(protocol="udp")))
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
