#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from collections.abc import Mapping, Sequence
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

from generated.test.Ice.scope import Test
from generated.test.Ice.scope.Inner.Test import Inner2 as Inner_Test_Inner2
from generated.test.Ice.scope.Test import Inner as Test_Inner
from generated.test.Ice.scope.Test.Inner import Inner2 as Test_Inner_Inner2

import Ice


class MyInterface1(Test.MyInterface):
    @override
    def opMyStruct(self, s1: Test.MyStruct, current: Ice.Current) -> tuple[Test.MyStruct, Test.MyStruct]:
        return (s1, s1)

    @override
    def opMyStructSeq(
        self, s1: list[Test.MyStruct], current: Ice.Current
    ) -> tuple[list[Test.MyStruct], list[Test.MyStruct]]:
        return (s1, s1)

    @override
    def opMyStructMap(
        self, s1: dict[str, Test.MyStruct], current: Ice.Current
    ) -> tuple[Mapping[str, Test.MyStruct], Mapping[str, Test.MyStruct]]:
        return (s1, s1)

    @override
    def opMyClass(
        self, c1: Test.MyClass | None, current: Ice.Current
    ) -> tuple[Test.MyClass | None, Test.MyClass | None]:
        return (c1, c1)

    @override
    def opMyClassSeq(
        self, s1: list[Test.MyClass | None], current: Ice.Current
    ) -> tuple[Sequence[Test.MyClass | None], Sequence[Test.MyClass | None]]:
        return (s1, s1)

    @override
    def opMyClassMap(
        self, c1: dict[str, Test.MyClass | None], current: Ice.Current
    ) -> tuple[Mapping[str, Test.MyClass | None], Mapping[str, Test.MyClass | None]]:
        return (c1, c1)

    @override
    def opMyEnum(self, e1: Test.MyEnum, current: Ice.Current) -> Test.MyEnum:
        return e1

    @override
    def opMyOtherStruct(self, s1: Test.MyOtherStruct, current: Ice.Current) -> Test.MyOtherStruct:
        return s1

    @override
    def opMyOtherClass(self, c1: Test.MyOtherClass | None, current: Ice.Current) -> Test.MyOtherClass | None:
        return c1

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class MyInterface2(Test_Inner.MyInterface):
    @override
    def opMyStruct(
        self, s1: Test_Inner_Inner2.MyStruct, current: Ice.Current
    ) -> tuple[Test_Inner_Inner2.MyStruct, Test_Inner_Inner2.MyStruct]:
        return (s1, s1)

    @override
    def opMyStructSeq(
        self, s1: list[Test_Inner_Inner2.MyStruct], current: Ice.Current
    ) -> tuple[Sequence[Test_Inner_Inner2.MyStruct], Sequence[Test_Inner_Inner2.MyStruct]]:
        return (s1, s1)

    @override
    def opMyStructMap(
        self, s1: dict[str, Test_Inner_Inner2.MyStruct], current: Ice.Current
    ) -> tuple[Mapping[str, Test_Inner_Inner2.MyStruct], Mapping[str, Test_Inner_Inner2.MyStruct]]:
        return (s1, s1)

    @override
    def opMyClass(
        self, c1: Test_Inner_Inner2.MyClass | None, current: Ice.Current
    ) -> tuple[Test_Inner_Inner2.MyClass | None, Test_Inner_Inner2.MyClass | None]:
        return (c1, c1)

    @override
    def opMyClassSeq(
        self, c1: list[Test_Inner_Inner2.MyClass | None], current: Ice.Current
    ) -> tuple[Sequence[Test_Inner_Inner2.MyClass | None], Sequence[Test_Inner_Inner2.MyClass | None]]:
        return (c1, c1)

    @override
    def opMyClassMap(
        self, c1: dict[str, Test_Inner_Inner2.MyClass | None], current: Ice.Current
    ) -> tuple[Mapping[str, Test_Inner_Inner2.MyClass | None], Mapping[str, Test_Inner_Inner2.MyClass | None]]:
        return (c1, c1)

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class MyInterface3(Test_Inner.MyInterface):
    @override
    def opMyStruct(
        self, s1: Test_Inner_Inner2.MyStruct, current: Ice.Current
    ) -> tuple[Test_Inner_Inner2.MyStruct, Test_Inner_Inner2.MyStruct]:
        return (s1, s1)

    @override
    def opMyStructSeq(
        self, s1: list[Test_Inner_Inner2.MyStruct], current: Ice.Current
    ) -> tuple[Sequence[Test_Inner_Inner2.MyStruct], Sequence[Test_Inner_Inner2.MyStruct]]:
        return (s1, s1)

    @override
    def opMyStructMap(
        self, s1: dict[str, Test_Inner_Inner2.MyStruct], current: Ice.Current
    ) -> tuple[Mapping[str, Test_Inner_Inner2.MyStruct], Mapping[str, Test_Inner_Inner2.MyStruct]]:
        return (s1, s1)

    @override
    def opMyClass(
        self, c1: Test_Inner_Inner2.MyClass | None, current: Ice.Current
    ) -> tuple[Test_Inner_Inner2.MyClass | None, Test_Inner_Inner2.MyClass | None]:
        return (c1, c1)

    @override
    def opMyClassSeq(
        self, c1: list[Test_Inner_Inner2.MyClass | None], current: Ice.Current
    ) -> tuple[Sequence[Test_Inner_Inner2.MyClass | None], Sequence[Test_Inner_Inner2.MyClass | None]]:
        return (c1, c1)

    @override
    def opMyClassMap(
        self, c1: dict[str, Test_Inner_Inner2.MyClass | None], current: Ice.Current
    ) -> tuple[Mapping[str, Test_Inner_Inner2.MyClass | None], Mapping[str, Test_Inner_Inner2.MyClass | None]]:
        return (c1, c1)

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class MyInterface4(Inner_Test_Inner2.MyInterface):
    @override
    def opMyStruct(self, s1: Test.MyStruct, current: Ice.Current) -> tuple[Test.MyStruct, Test.MyStruct]:
        return (s1, s1)

    @override
    def opMyStructSeq(
        self, s1: list[Test.MyStruct], current: Ice.Current
    ) -> tuple[Sequence[Test.MyStruct], Sequence[Test.MyStruct]]:
        return (s1, s1)

    @override
    def opMyStructMap(
        self, s1: dict[str, Test.MyStruct], current: Ice.Current
    ) -> tuple[Mapping[str, Test.MyStruct], Mapping[str, Test.MyStruct]]:
        return (s1, s1)

    @override
    def opMyClass(
        self, c1: Test.MyClass | None, current: Ice.Current
    ) -> tuple[Test.MyClass | None, Test.MyClass | None]:
        return (c1, c1)

    @override
    def opMyClassSeq(
        self, c1: list[Test.MyClass | None], current: Ice.Current
    ) -> tuple[Sequence[Test.MyClass | None], Sequence[Test.MyClass | None]]:
        return (c1, c1)

    @override
    def opMyClassMap(
        self, c1: dict[str, Test.MyClass | None], current: Ice.Current
    ) -> tuple[Mapping[str, Test.MyClass | None], Mapping[str, Test.MyClass | None]]:
        return (c1, c1)

    @override
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
