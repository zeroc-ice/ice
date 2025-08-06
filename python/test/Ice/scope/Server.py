#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from generated.test.Ice.scope import Test
from generated.test.Ice.scope.Inner.Test.Inner2 import MyInterface as Inner_Test_Inner2_MyInterface
from generated.test.Ice.scope.Test.Inner import MyInterface as Test_Inner_MyInterface
from generated.test.Ice.scope.Test.Inner.Inner2 import MyInterface as Test_Inner_Inner2_MyInterface

import Ice


class MyInterface1(Test.MyInterface):
    def opMyStruct(self, s1, current: Ice.Current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current: Ice.Current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current: Ice.Current):
        return (smap1, smap1)

    def opMyClass(self, c1, current: Ice.Current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current: Ice.Current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current: Ice.Current):
        return (cmap1, cmap1)

    def opMyEnum(self, e1, current: Ice.Current):
        return e1

    def opMyOtherStruct(self, s1, current: Ice.Current):
        return s1

    def opMyOtherClass(self, c1, current: Ice.Current):
        return c1

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class MyInterface2(Test_Inner_Inner2_MyInterface):
    def opMyStruct(self, s1, current: Ice.Current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current: Ice.Current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current: Ice.Current):
        return (smap1, smap1)

    def opMyClass(self, c1, current: Ice.Current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current: Ice.Current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current: Ice.Current):
        return (cmap1, cmap1)

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class MyInterface3(Test_Inner_MyInterface):
    def opMyStruct(self, s1, current: Ice.Current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current: Ice.Current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current: Ice.Current):
        return (smap1, smap1)

    def opMyClass(self, c1, current: Ice.Current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current: Ice.Current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current: Ice.Current):
        return (cmap1, cmap1)

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class MyInterface4(Inner_Test_Inner2_MyInterface):
    def opMyStruct(self, s1, current: Ice.Current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current: Ice.Current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current: Ice.Current):
        return (smap1, smap1)

    def opMyClass(self, c1, current: Ice.Current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current: Ice.Current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current: Ice.Current):
        return (cmap1, cmap1)

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args: list[str]):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(MyInterface1(), Ice.stringToIdentity("i1"))
            adapter.add(MyInterface2(), Ice.stringToIdentity("i2"))
            adapter.add(MyInterface3(), Ice.stringToIdentity("i3"))
            adapter.add(MyInterface4(), Ice.stringToIdentity("i4"))
            adapter.activate()
            communicator.waitForShutdown()
