#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import concurrent.futures
import threading
import time
from collections.abc import Awaitable, Mapping, Sequence
from typing import Any, override

from generated.test.Ice.operations import Test
from TestHelper import test

import Ice


class FutureThread(threading.Thread):
    def __init__(self, f: Ice.Future | concurrent.futures.Future, r: Any):
        threading.Thread.__init__(self)
        self.future = f
        self.result = r

    def run(self):
        time.sleep(0.05)
        self.future.set_result(self.result)


class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.threads = []
        self.threadLock = threading.Lock()
        self.lock = threading.Lock()
        self.opByteSOnewayCount = 0

    @override
    def shutdown(self, current: Ice.Current) -> Awaitable[None]:
        with self.threadLock:
            for thread in self.threads:
                thread.join()
            self.threads = []

        current.adapter.getCommunicator().shutdown()
        return Ice.Future.completed(None)

    @override
    def supportsCompress(self, current: Ice.Current) -> Awaitable[bool]:
        return Ice.Future.completed(True)

    @override
    def opVoid(self, current: Ice.Current) -> Awaitable[None]:
        test(current.mode == Ice.OperationMode.Normal)

        f = Ice.Future()

        with self.threadLock:
            thread = FutureThread(f, None)
            self.threads.append(thread)
            thread.start()

        return f

    @override
    def opByte(self, p1: int, p2: int, current: Ice.Current) -> Any:
        # Test the ability to use another Future type
        f = concurrent.futures.Future()
        with self.threadLock:
            thread = FutureThread(f, (p1, p1 ^ p2))
            self.threads.append(thread)
            thread.start()
        return f

    @override
    def opBool(self, p1: bool, p2: bool, current: Ice.Current) -> Awaitable[tuple[bool, bool]]:
        return Ice.Future.completed((p2, p1))

    # Test the ability to define a servant method as a coroutine
    @override
    async def opShortIntLong(self, p1: int, p2: int, p3: int, current: Ice.Current) -> tuple[int, int, int, int]:
        f = Ice.Future()

        with self.threadLock:
            thread = FutureThread(f, (p3, p1, p2, p3))
            self.threads.append(thread)
            thread.start()

        return await f

    @override
    def opFloatDouble(self, p1: float, p2: float, current: Ice.Current) -> Awaitable[tuple[float, float, float]]:
        return Ice.Future.completed((p2, p1, p2))

    @override
    def opString(self, p1: str, p2: str, current: Ice.Current) -> Awaitable[tuple[str, str]]:
        return Ice.Future.completed((p1 + " " + p2, p2 + " " + p1))

    @override
    def opMyEnum(self, p1: Test.MyEnum, current: Ice.Current) -> Awaitable[tuple[Test.MyEnum, Test.MyEnum]]:
        return Ice.Future.completed((Test.MyEnum.enum3, p1))

    @override
    def opMyClass(
        self, p1: Test.MyClassPrx | None, current: Ice.Current
    ) -> Awaitable[tuple[Test.MyClassPrx | None, Test.MyClassPrx | None, Test.MyClassPrx | None]]:
        p2 = p1
        p3 = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")))
        return Ice.Future.completed(
            (
                Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id)),
                p2,
                p3,
            )
        )

    @override
    def opStruct(
        self, p1: Test.Structure, p2: Test.Structure, current: Ice.Current
    ) -> Awaitable[tuple[Test.Structure, Test.Structure]]:
        p1.s.s = "a new string"
        return Ice.Future.completed((p2, p1))

    @override
    def opByteS(self, p1: bytes, p2: bytes, current: Ice.Current) -> Awaitable[tuple[bytes, bytes]]:
        p3 = bytes(reversed(p1))
        r = p1 + p2
        return Ice.Future.completed((r, p3))

    @override
    def opBoolS(self, p1: list[bool], p2: list[bool], current: Ice.Current) -> Awaitable[tuple[list[bool], list[bool]]]:
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    @override
    def opShortIntLongS(
        self, p1: list[int], p2: list[int], p3: list[int], current: Ice.Current
    ) -> Awaitable[tuple[list[int], list[int], list[int], list[int]]]:
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return Ice.Future.completed((p3, p4, p5, p6))

    @override
    def opFloatDoubleS(
        self, p1: list[float], p2: list[float], current: Ice.Current
    ) -> Awaitable[tuple[list[float], list[float], list[float]]]:
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p1)
        return Ice.Future.completed((r, p3, p4))

    @override
    def opStringS(self, p1: list[str], p2: list[str], current: Ice.Current) -> Awaitable[tuple[list[str], list[str]]]:
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    @override
    def opByteSS(
        self, p1: list[bytes], p2: list[bytes], current: Ice.Current
    ) -> Awaitable[tuple[list[bytes], list[bytes]]]:
        p3 = p1[0:]
        p3.reverse()
        r = p1[0:]
        r.extend(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opBoolSS(
        self, p1: list[list[bool]], p2: list[list[bool]], current: Ice.Current
    ) -> Awaitable[tuple[list[list[bool]], list[list[bool]]]]:
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    @override
    def opShortIntLongSS(
        self, p1: list[list[int]], p2: list[list[int]], p3: list[list[int]], current: Ice.Current
    ) -> Awaitable[tuple[list[list[int]], list[list[int]], list[list[int]], list[list[int]]]]:
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return Ice.Future.completed((p3, p4, p5, p6))

    @override
    def opFloatDoubleSS(
        self, p1: list[list[float]], p2: list[list[float]], current: Ice.Current
    ) -> Awaitable[tuple[list[list[float]], list[list[float]], list[list[float]]]]:
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p2)
        return Ice.Future.completed((r, p3, p4))

    @override
    def opStringSS(
        self, p1: list[list[str]], p2: list[list[str]], current: Ice.Current
    ) -> Awaitable[tuple[list[list[str]], list[list[str]]]]:
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    @override
    def opStringSSS(
        self, p1: list[list[list[str]]], p2: list[list[list[str]]], current: Ice.Current
    ) -> Awaitable[tuple[list[list[list[str]]], list[list[list[str]]]]]:
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    @override
    def opByteBoolD(
        self, p1: dict[int, bool], p2: dict[int, bool], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, bool], Mapping[int, bool]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opShortIntD(
        self, p1: dict[int, int], p2: dict[int, int], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, int], Mapping[int, int]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opLongFloatD(
        self, p1: dict[int, float], p2: dict[int, float], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, float], Mapping[int, float]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opStringStringD(
        self, p1: dict[str, str], p2: dict[str, str], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[str, str], Mapping[str, str]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opStringMyEnumD(
        self, p1: dict[str, Test.MyEnum], p2: dict[str, Test.MyEnum], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[str, Test.MyEnum], Mapping[str, Test.MyEnum]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opMyEnumStringD(
        self, p1: dict[Test.MyEnum, str], p2: dict[Test.MyEnum, str], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[Test.MyEnum, str], Mapping[Test.MyEnum, str]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opMyStructMyEnumD(
        self, p1: dict[Test.MyStruct, Test.MyEnum], p2: dict[Test.MyStruct, Test.MyEnum], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[Test.MyStruct, Test.MyEnum], Mapping[Test.MyStruct, Test.MyEnum]]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opByteBoolDS(
        self, p1: list[dict[int, bool]], p2: list[dict[int, bool]], current: Ice.Current
    ) -> Awaitable[tuple[Sequence[Mapping[int, bool]], Sequence[Mapping[int, bool]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opShortIntDS(
        self, p1: list[dict[int, int]], p2: list[dict[int, int]], current: Ice.Current
    ) -> Awaitable[tuple[Sequence[Mapping[int, int]], Sequence[Mapping[int, int]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opLongFloatDS(
        self, p1: list[dict[int, float]], p2: list[dict[int, float]], current: Ice.Current
    ) -> Awaitable[tuple[Sequence[Mapping[int, float]], Sequence[Mapping[int, float]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opStringStringDS(
        self, p1: list[dict[str, str]], p2: list[dict[str, str]], current: Ice.Current
    ) -> Awaitable[tuple[Sequence[Mapping[str, str]], Sequence[Mapping[str, str]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opStringMyEnumDS(
        self, p1: list[dict[str, Test.MyEnum]], p2: list[dict[str, Test.MyEnum]], current: Ice.Current
    ) -> Awaitable[tuple[Sequence[Mapping[str, Test.MyEnum]], Sequence[Mapping[str, Test.MyEnum]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opMyEnumStringDS(
        self, p1: list[dict[Test.MyEnum, str]], p2: list[dict[Test.MyEnum, str]], current: Ice.Current
    ) -> Awaitable[tuple[Sequence[Mapping[Test.MyEnum, str]], Sequence[Mapping[Test.MyEnum, str]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opMyStructMyEnumDS(
        self,
        p1: list[dict[Test.MyStruct, Test.MyEnum]],
        p2: list[dict[Test.MyStruct, Test.MyEnum]],
        current: Ice.Current,
    ) -> Awaitable[tuple[Sequence[Mapping[Test.MyStruct, Test.MyEnum]], Sequence[Mapping[Test.MyStruct, Test.MyEnum]]]]:
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    @override
    def opByteByteSD(
        self, p1: dict[int, bytes], p2: dict[int, bytes], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, bytes], Mapping[int, bytes]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opBoolBoolSD(
        self, p1: dict[bool, list[bool]], p2: dict[bool, list[bool]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[bool, list[bool]], Mapping[bool, list[bool]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opShortShortSD(
        self, p1: dict[int, list[int]], p2: dict[int, list[int]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, list[int]], Mapping[int, list[int]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opIntIntSD(
        self, p1: dict[int, list[int]], p2: dict[int, list[int]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, list[int]], Mapping[int, list[int]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opLongLongSD(
        self, p1: dict[int, list[int]], p2: dict[int, list[int]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[int, list[int]], Mapping[int, list[int]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opStringFloatSD(
        self, p1: dict[str, list[float]], p2: dict[str, list[float]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[str, list[float]], Mapping[str, list[float]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opStringDoubleSD(
        self, p1: dict[str, list[float]], p2: dict[str, list[float]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[str, list[float]], Mapping[str, list[float]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opStringStringSD(
        self, p1: dict[str, list[str]], p2: dict[str, list[str]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[str, list[str]], Mapping[str, list[str]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opMyEnumMyEnumSD(
        self, p1: dict[Test.MyEnum, list[Test.MyEnum]], p2: dict[Test.MyEnum, list[Test.MyEnum]], current: Ice.Current
    ) -> Awaitable[tuple[Mapping[Test.MyEnum, list[Test.MyEnum]], Mapping[Test.MyEnum, list[Test.MyEnum]]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    @override
    def opIntS(self, s: list[int], current: Ice.Current) -> Awaitable[Sequence[int]]:
        return Ice.Future.completed([-x for x in s])

    @override
    def opByteSOneway(self, s: bytes, current: Ice.Current) -> Awaitable[None]:
        with self.lock:
            self.opByteSOnewayCount += 1
        return Ice.Future.completed(None)

    @override
    def opByteSOnewayCallCount(self, current: Ice.Current) -> Awaitable[int]:
        with self.lock:
            count = self.opByteSOnewayCount
            self.opByteSOnewayCount = 0
        return Ice.Future.completed(count)

    @override
    def opDoubleMarshaling(self, p1: float, p2: list[float], current: Ice.Current) -> Awaitable[None]:
        d = 1278312346.0 / 13.0
        test(p1 == d)
        for i in p2:
            test(i == d)
        return Ice.Future.completed(None)

    @override
    def opContext(self, current: Ice.Current) -> Awaitable[Mapping[str, str]]:
        return Ice.Future.completed(current.ctx)

    @override
    def opIdempotent(self, current: Ice.Current) -> Awaitable[None]:
        test(current.mode == Ice.OperationMode.Idempotent)
        return Ice.Future.completed(None)

    @override
    def opDerived(self, current: Ice.Current) -> Awaitable[None]:
        return Ice.Future.completed(None)

    @override
    def opByte1(self, opByte1: int, current: Ice.Current) -> Awaitable[int]:
        return Ice.Future.completed(opByte1)

    @override
    def opShort1(self, opShort1: int, current: Ice.Current) -> Awaitable[int]:
        return Ice.Future.completed(opShort1)

    @override
    def opInt1(self, opInt1: int, current: Ice.Current) -> Awaitable[int]:
        return Ice.Future.completed(opInt1)

    @override
    def opLong1(self, opLong1: int, current: Ice.Current) -> Awaitable[int]:
        return Ice.Future.completed(opLong1)

    @override
    def opFloat1(self, opFloat1: float, current: Ice.Current) -> Awaitable[float]:
        return Ice.Future.completed(opFloat1)

    @override
    def opDouble1(self, opDouble1: float, current: Ice.Current) -> Awaitable[float]:
        return Ice.Future.completed(opDouble1)

    @override
    def opString1(self, opString1: str, current: Ice.Current) -> Awaitable[str]:
        return Ice.Future.completed(opString1)

    @override
    def opStringS1(self, opStringS1: list[str], current: Ice.Current) -> Awaitable[Sequence[str]]:
        return Ice.Future.completed(opStringS1)

    @override
    def opByteBoolD1(self, opByteBoolD1: dict[int, bool], current: Ice.Current) -> Awaitable[Mapping[int, bool]]:
        return Ice.Future.completed(opByteBoolD1)

    @override
    def opStringS2(self, stringS: list[str], current: Ice.Current) -> Awaitable[Sequence[str]]:
        return Ice.Future.completed(stringS)

    @override
    def opByteBoolD2(self, byteBoolD: dict[int, bool], current: Ice.Current) -> Awaitable[Mapping[int, bool]]:
        return Ice.Future.completed(byteBoolD)

    @override
    def opMyClass1(self, opMyClass1: Test.MyClass1 | None, current: Ice.Current) -> Awaitable[Test.MyClass1 | None]:
        return Ice.Future.completed(opMyClass1)

    @override
    def opMyStruct1(self, opMyStruct1: Test.MyStruct1, current: Ice.Current) -> Awaitable[Test.MyStruct1]:
        return Ice.Future.completed(opMyStruct1)

    @override
    def opStringLiterals(self, current: Ice.Current) -> Awaitable[Sequence[str]]:
        return Ice.Future.completed(
            [
                Test.s0,
                Test.s1,
                Test.s2,
                Test.s3,
                Test.s4,
                Test.s5,
                Test.s6,
                Test.s7,
                Test.s8,
                Test.s9,
                Test.s10,
                Test.sw0,
                Test.sw1,
                Test.sw2,
                Test.sw3,
                Test.sw4,
                Test.sw5,
                Test.sw6,
                Test.sw7,
                Test.sw8,
                Test.sw9,
                Test.sw10,
                Test.ss0,
                Test.ss1,
                Test.ss2,
                Test.ss3,
                Test.ss4,
                Test.ss5,
                Test.su0,
                Test.su1,
                Test.su2,
            ]
        )

    @override
    def opWStringLiterals(self, current: Ice.Current) -> Awaitable[Sequence[str]]:
        return self.opStringLiterals(current)

    @override
    def opMStruct1(self, current: Ice.Current):
        return Ice.Future.completed(Test.Structure())

    @override
    def opMStruct2(self, p1: Test.Structure, current: Ice.Current):
        return Ice.Future.completed((p1, p1))

    @override
    def opMSeq1(self, current: Ice.Current):
        return Ice.Future.completed([])

    @override
    def opMSeq2(self, p1: list[str], current: Ice.Current):
        return Ice.Future.completed((p1, p1))

    @override
    def opMDict1(self, current: Ice.Current):
        return Ice.Future.completed({})

    @override
    def opMDict2(self, p1: dict[str, str], current: Ice.Current):
        return Ice.Future.completed((p1, p1))
