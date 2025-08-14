#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from collections.abc import Awaitable, Mapping, Sequence
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from generated.test.Ice.optional import Test

import Ice


class InitialI(Test.Initial):
    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    @override
    def pingPong(self, o: Ice.Value | None, current: Ice.Current) -> Awaitable[Ice.Value | None]:
        return Ice.Future.completed(o)

    @override
    def opOptionalException(self, a: int | None, b: str | None, current: Ice.Current):
        f = Ice.Future()
        f.set_exception(Test.OptionalException(False, a, b))
        return f

    @override
    def opDerivedException(self, a: int | None, b: str | None, current: Ice.Current):
        f = Ice.Future()
        f.set_exception(Test.DerivedException(False, a, b, "d1", b, "d2"))
        return f

    @override
    def opRequiredException(self, a: int | None, b: str | None, current: Ice.Current):
        e = Test.RequiredException()
        e.a = a
        e.b = b
        if b is not None:
            e.ss = b
        f = Ice.Future()
        f.set_exception(e)
        return f

    @override
    def opByte(self, p1: int | None, current: Ice.Current) -> Awaitable[tuple[int | None, int | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opBool(self, p1: bool | None, current: Ice.Current) -> Awaitable[tuple[bool | None, bool | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opShort(self, p1: int | None, current: Ice.Current) -> Awaitable[tuple[int | None, int | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opInt(self, p1: int | None, current: Ice.Current) -> Awaitable[tuple[int | None, int | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opLong(self, p1: int | None, current: Ice.Current) -> Awaitable[tuple[int | None, int | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opFloat(self, p1: float | None, current: Ice.Current) -> Awaitable[tuple[float | None, float | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opDouble(self, p1: float | None, current: Ice.Current) -> Awaitable[tuple[float | None, float | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opString(self, p1: str | None, current: Ice.Current) -> Awaitable[tuple[str | None, str | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opMyEnum(
        self, p1: Test.MyEnum | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.MyEnum | None, Test.MyEnum | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opSmallStruct(
        self, p1: Test.SmallStruct | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.SmallStruct | None, Test.SmallStruct | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opFixedStruct(
        self, p1: Test.FixedStruct | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.FixedStruct | None, Test.FixedStruct | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opVarStruct(
        self, p1: Test.VarStruct | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.VarStruct | None, Test.VarStruct | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opOneOptional(
        self, p1: Test.OneOptional | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.OneOptional | None, Test.OneOptional | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opMyInterfaceProxy(
        self, p1: Test.MyInterfacePrx | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.MyInterfacePrx | None, Test.MyInterfacePrx | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opByteSeq(
        self, p1: bytes | None, current: Ice.Current
    ) -> Awaitable[tuple[Sequence[int] | None, Sequence[int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opBoolSeq(
        self, p1: list[bool] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[bool] | None, list[bool] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opShortSeq(
        self, p1: list[int] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[int] | None, list[int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opIntSeq(
        self, p1: list[int] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[int] | None, list[int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opLongSeq(
        self, p1: list[int] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[int] | None, list[int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opFloatSeq(
        self, p1: list[float] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[float] | None, list[float] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opDoubleSeq(
        self, p1: list[float] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[float] | None, list[float] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opStringSeq(
        self, p1: list[str] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[str] | None, list[str] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opSmallStructSeq(
        self, p1: list[Test.SmallStruct] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[Test.SmallStruct] | None, list[Test.SmallStruct] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opSmallStructList(
        self, p1: list[Test.SmallStruct] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[Test.SmallStruct] | None, list[Test.SmallStruct] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opFixedStructSeq(
        self, p1: list[Test.FixedStruct] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[Test.FixedStruct] | None, list[Test.FixedStruct] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opFixedStructList(
        self, p1: list[Test.FixedStruct] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[Test.FixedStruct] | None, list[Test.FixedStruct] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opVarStructSeq(
        self, p1: list[Test.VarStruct] | None, current: Ice.Current
    ) -> Awaitable[tuple[list[Test.VarStruct] | None, list[Test.VarStruct] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opSerializable(self, p1: bytes | None, current: Ice.Current) -> Awaitable[tuple[bytes | None, bytes | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opIntIntDict(
        self, p1: dict[int, int] | None, current: Ice.Current
    ) -> Awaitable[tuple[dict[int, int] | None, dict[int, int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opStringIntDict(
        self, p1: dict[str, int] | None, current: Ice.Current
    ) -> Awaitable[tuple[dict[str, int] | None, dict[str, int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opClassAndUnknownOptional(self, p: Test.A | None, current: Ice.Current):
        return Ice.Future.completed(None)

    @override
    def opG(self, g: Test.G | None, current: Ice.Current) -> Awaitable[Test.G | None]:
        return Ice.Future.completed(g)

    @override
    def opVoid(self, current: Ice.Current) -> Awaitable[None]:
        return Ice.Future.completed(None)

    @override
    def opMStruct1(self, current: Ice.Current) -> Awaitable[Test.SmallStruct | None]:
        return Ice.Future.completed(Test.SmallStruct())

    @override
    def opMStruct2(
        self, p1: Test.SmallStruct | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.SmallStruct | None, Test.SmallStruct | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opMSeq1(self, current: Ice.Current) -> Awaitable[list[str] | None]:
        return Ice.Future.completed([])

    @override
    def opMSeq2(
        self, p1: list[str] | None, current: Ice.Current
    ) -> Awaitable[tuple[Sequence[str] | None, Sequence[str] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opMDict1(self, current: Ice.Current):
        return Ice.Future.completed({})

    @override
    def opMDict2(
        self, p1: dict[str, int] | None, current: Ice.Current
    ) -> Awaitable[tuple[Mapping[str, int] | None, Mapping[str, int] | None]]:
        return Ice.Future.completed((p1, p1))

    @override
    def opRequiredAfterOptional(
        self, p1: int, p2: int | None, p3: int, current: Ice.Current
    ) -> Awaitable[tuple[int, int | None, int]]:
        return Ice.Future.completed((p1, p2, p3))

    @override
    def opOptionalAfterRequired(
        self, p1: int, p2: int | None, p3: int | None, current: Ice.Current
    ) -> Awaitable[tuple[int, int | None, int | None]]:
        return Ice.Future.completed((p1, p2, p3))

    @override
    def supportsJavaSerializable(self, current: Ice.Current):
        return Ice.Future.completed(True)


class ServerAMD(TestHelper):
    @override
    def run(self, args: list[str]):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = InitialI()
            adapter.add(initial, Ice.stringToIdentity("initial"))
            adapter.activate()

            communicator.waitForShutdown()
