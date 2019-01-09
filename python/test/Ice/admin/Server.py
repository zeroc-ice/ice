#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI


class Server(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.RemoteCommunicatorFactoryI(), Ice.stringToIdentity("factory"))
            adapter.activate()

            communicator.waitForShutdown()
