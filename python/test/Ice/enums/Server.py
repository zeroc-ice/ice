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
TestHelper.loadSlice("Test.ice")
import Ice
import Test


class TestIntfI(Test.TestIntf):
    def opByte(self, b1, current=None):
        return (b1, b1)

    def opShort(self, s1, current=None):
        return (s1, s1)

    def opInt(self, i1, current=None):
        return (i1, i1)

    def opSimple(self, s1, current=None):
        return (s1, s1)

    def opByteSeq(self, b1, current=None):
        return (b1, b1)

    def opShortSeq(self, s1, current=None):
        return (s1, s1)

    def opIntSeq(self, i1, current=None):
        return (i1, i1)

    def opSimpleSeq(self, s1, current=None):
        return (s1, s1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):

        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestIntfI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
