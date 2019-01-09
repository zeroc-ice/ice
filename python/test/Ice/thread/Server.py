#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Ice
import TestI


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                     "{0} -t 10000".format(self.getTestEndpoint()))
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.RemoteCommunicatorFactoryI(), Ice.stringToIdentity("factory"))
            adapter.activate()
            communicator.waitForShutdown()
