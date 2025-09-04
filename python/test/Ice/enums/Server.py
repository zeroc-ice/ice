#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

from generated.test.Ice.enums import Test

import Ice


class TestIntfI(Test.TestIntf):
    @override
    def opByte(self, b1: Test.ByteEnum, current: Ice.Current):
        return (b1, b1)

    @override
    def opShort(self, s1: Test.ShortEnum, current: Ice.Current):
        return (s1, s1)

    @override
    def opInt(self, i1: Test.IntEnum, current: Ice.Current):
        return (i1, i1)

    @override
    def opSimple(self, s1: Test.SimpleEnum, current: Ice.Current):
        return (s1, s1)

    @override
    def opByteSeq(self, b1: list[Test.ByteEnum], current: Ice.Current):
        return (b1, b1)

    @override
    def opShortSeq(self, s1: list[Test.ShortEnum], current: Ice.Current):
        return (s1, s1)

    @override
    def opIntSeq(self, i1: list[Test.IntEnum], current: Ice.Current):
        return (i1, i1)

    @override
    def opSimpleSeq(self, s1: list[Test.SimpleEnum], current: Ice.Current):
        return (s1, s1)

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args: list[str]):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestIntfI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
