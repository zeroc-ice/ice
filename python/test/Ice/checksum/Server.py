#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("--checksum Test.ice STypes.ice")
import Test


class ChecksumI(Test.Checksum):
    def getSliceChecksums(self, current=None):
        return Ice.sliceChecksums

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):

        with self.initialize(args=args) as communicator:

            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(ChecksumI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
