#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper
import Test
import Inner
import Ice


class MyInterface1(Test.MyInterface):
    def opMyStruct(self, s1, current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current):
        return (smap1, smap1)

    def opMyClass(self, c1, current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current):
        return (cmap1, cmap1)

    def opMyEnum(self, e1, current):
        return e1

    def opMyOtherStruct(self, s1, current):
        return s1

    def opMyOtherClass(self, c1, current):
        return c1

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class MyInterface2(Test.Inner.Inner2.MyInterface):
    def opMyStruct(self, s1, current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current):
        return (smap1, smap1)

    def opMyClass(self, c1, current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current):
        return (cmap1, cmap1)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class MyInterface3(Test.Inner.MyInterface):
    def opMyStruct(self, s1, current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current):
        return (smap1, smap1)

    def opMyClass(self, c1, current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current):
        return (cmap1, cmap1)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class MyInterface4(Inner.Test.Inner2.MyInterface):
    def opMyStruct(self, s1, current):
        return (s1, s1)

    def opMyStructSeq(self, sseq1, current):
        return (sseq1, sseq1)

    def opMyStructMap(self, smap1, current):
        return (smap1, smap1)

    def opMyClass(self, c1, current):
        return (c1, c1)

    def opMyClassSeq(self, cseq1, current):
        return (cseq1, cseq1)

    def opMyClassMap(self, cmap1, current):
        return (cmap1, cmap1)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(MyInterface1(), Ice.stringToIdentity("i1"))
            adapter.add(MyInterface2(), Ice.stringToIdentity("i2"))
            adapter.add(MyInterface3(), Ice.stringToIdentity("i3"))
            adapter.add(MyInterface4(), Ice.stringToIdentity("i4"))
            adapter.activate()
            communicator.waitForShutdown()
