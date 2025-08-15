# Copyright (c) ZeroC, Inc.

import threading
from collections.abc import Mapping, Sequence
from typing import override

from generated.test.Ice.operations import Test
from TestHelper import test

import Ice


class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.lock = threading.Lock()
        self.opByteSOnewayCount = 0

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    @override
    def supportsCompress(self, current: Ice.Current):
        return True

    @override
    def opVoid(self, current: Ice.Current):
        test(current.mode == Ice.OperationMode.Normal)

    @override
    def opByte(self, p1: int, p2: int, current: Ice.Current) -> tuple[int, int]:
        return (p1, p1 ^ p2)

    @override
    def opBool(self, p1: bool, p2: bool, current: Ice.Current) -> tuple[bool, bool]:
        return (p2, p1)

    @override
    def opShortIntLong(self, p1: int, p2: int, p3: int, current: Ice.Current) -> tuple[int, int, int, int]:
        return (p3, p1, p2, p3)

    @override
    def opFloatDouble(self, p1: float, p2: float, current: Ice.Current) -> tuple[float, float, float]:
        return (p2, p1, p2)

    @override
    def opString(self, p1: str, p2: str, current: Ice.Current) -> tuple[str, str]:
        return (p1 + " " + p2, p2 + " " + p1)

    @override
    def opMyEnum(self, p1: Test.MyEnum, current: Ice.Current) -> tuple[Test.MyEnum, Test.MyEnum]:
        return (Test.MyEnum.enum3, p1)

    def opMyClass(
        self, p1: Test.MyClassPrx | None, current: Ice.Current
    ) -> tuple[Test.MyClassPrx | None, Test.MyClassPrx | None, Test.MyClassPrx | None]:
        return (
            Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id)),
            p1,
            Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity"))),
        )

    @override
    def opStruct(
        self, p1: Test.Structure, p2: Test.Structure, current: Ice.Current
    ) -> tuple[Test.Structure, Test.Structure]:
        p1.s.s = "a new string"
        return (p2, p1)

    @override
    def opByteS(self, p1: bytes, p2: bytes, current: Ice.Current) -> tuple[bytes, bytes]:
        p3 = bytes(reversed(p1))
        r = p1 + p2
        return (r, p3)

    @override
    def opBoolS(self, p1: list[bool], p2: list[bool], current: Ice.Current) -> tuple[Sequence[bool], Sequence[bool]]:
        p3 = list(p1)
        p3.extend(p2)
        r = list(p1)
        r.reverse()
        return (r, p3)

    @override
    def opShortIntLongS(
        self, p1: list[int], p2: list[int], p3: list[int], current: Ice.Current
    ) -> tuple[Sequence[int], Sequence[int], Sequence[int], Sequence[int]]:
        p4 = list(p1)
        p5 = list(p2)
        p5.reverse()
        p6 = list(p3)
        p6.extend(p3)
        return (p3, p4, p5, p6)

    @override
    def opFloatDoubleS(
        self, p1: list[float], p2: list[float], current: Ice.Current
    ) -> tuple[Sequence[float], Sequence[float], Sequence[float]]:
        p3 = list(p1)
        p4 = list(p2)
        p4.reverse()
        r = list(p2)
        r.extend(p1)
        return (r, p3, p4)

    @override
    def opStringS(self, p1: list[str], p2: list[str], current: Ice.Current) -> tuple[Sequence[str], Sequence[str]]:
        p3 = list(p1)
        p3.extend(p2)
        r = list(p1)
        r.reverse()
        return (r, p3)

    @override
    def opByteSS(
        self, p1: list[bytes], p2: list[bytes], current: Ice.Current
    ) -> tuple[Sequence[bytes], Sequence[bytes]]:
        p3 = list(p1)
        p3.reverse()
        r = list(p1)
        r.extend(p2)
        return (r, p3)

    @override
    def opBoolSS(
        self, p1: list[list[bool]], p2: list[list[bool]], current: Ice.Current
    ) -> tuple[Sequence[Sequence[bool]], Sequence[Sequence[bool]]]:
        p3 = list(p1)
        p3.extend(p2)
        r = list(p1)
        r.reverse()
        return (r, p3)

    @override
    def opShortIntLongSS(
        self,
        p1: list[list[int]],
        p2: list[list[int]],
        p3: list[list[int]],
        current: Ice.Current,
    ) -> tuple[Sequence[Sequence[int]], Sequence[Sequence[int]], Sequence[Sequence[int]], Sequence[Sequence[int]]]:
        p4 = list(p1)
        p5 = list(p2)
        p5.reverse()
        p6 = list(p3)
        p6.extend(p3)
        return (p3, p4, p5, p6)

    @override
    def opFloatDoubleSS(self, p1: list[list[float]], p2: list[list[float]], current: Ice.Current):
        p3 = list(p1)
        p4 = list(p2)
        p4.reverse()
        r = list(p2)
        r.extend(p2)
        return (r, p3, p4)

    @override
    def opStringSS(
        self, p1: list[list[str]], p2: list[list[str]], current: Ice.Current
    ) -> tuple[Sequence[Sequence[str]], Sequence[Sequence[str]]]:
        p3 = list(p1)
        p3.extend(p2)
        r = list(p2)
        r.reverse()
        return (r, p3)

    @override
    def opStringSSS(
        self, p1: list[list[list[str]]], p2: list[list[list[str]]], current: Ice.Current
    ) -> tuple[Sequence[Sequence[Sequence[str]]], Sequence[Sequence[Sequence[str]]]]:
        p3 = list(p1)
        p3.extend(p2)
        r = list(p2)
        r.reverse()
        return (r, p3)

    @override
    def opByteBoolD(
        self, p1: dict[int, bool], p2: dict[int, bool], current: Ice.Current
    ) -> tuple[Mapping[int, bool], Mapping[int, bool]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opShortIntD(
        self, p1: dict[int, int], p2: dict[int, int], current: Ice.Current
    ) -> tuple[Mapping[int, int], Mapping[int, int]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opLongFloatD(
        self, p1: dict[int, float], p2: dict[int, float], current: Ice.Current
    ) -> tuple[Mapping[int, float], Mapping[int, float]]:
        p3 = dict(p1)
        r = dict(p1)
        r.update(p2)
        return (r, p3)

    @override
    def opStringStringD(self, p1: dict[str, str], p2: dict[str, str], current: Ice.Current):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opStringMyEnumD(
        self, p1: dict[str, Test.MyEnum], p2: dict[str, Test.MyEnum], current: Ice.Current
    ) -> tuple[Mapping[str, Test.MyEnum], Mapping[str, Test.MyEnum]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opMyEnumStringD(
        self, p1: dict[Test.MyEnum, str], p2: dict[Test.MyEnum, str], current: Ice.Current
    ) -> tuple[Mapping[Test.MyEnum, str], Mapping[Test.MyEnum, str]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opMyStructMyEnumD(
        self, p1: dict[Test.MyStruct, Test.MyEnum], p2: dict[Test.MyStruct, Test.MyEnum], current: Ice.Current
    ) -> tuple[Mapping[Test.MyStruct, Test.MyEnum], Mapping[Test.MyStruct, Test.MyEnum]]:
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opByteBoolDS(
        self, p1: list[dict[int, bool]], p2: list[dict[int, bool]], current: Ice.Current
    ) -> tuple[Sequence[Mapping[int, bool]], Sequence[Mapping[int, bool]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opShortIntDS(
        self, p1: list[dict[int, int]], p2: list[dict[int, int]], current: Ice.Current
    ) -> tuple[Sequence[Mapping[int, int]], Sequence[Mapping[int, int]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opLongFloatDS(
        self, p1: list[dict[int, float]], p2: list[dict[int, float]], current: Ice.Current
    ) -> tuple[Sequence[Mapping[int, float]], Sequence[Mapping[int, float]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opStringStringDS(
        self, p1: list[dict[str, str]], p2: list[dict[str, str]], current: Ice.Current
    ) -> tuple[Sequence[Mapping[str, str]], Sequence[Mapping[str, str]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opStringMyEnumDS(
        self, p1: list[dict[str, Test.MyEnum]], p2: list[dict[str, Test.MyEnum]], current: Ice.Current
    ) -> tuple[Sequence[Mapping[str, Test.MyEnum]], Sequence[Mapping[str, Test.MyEnum]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opMyEnumStringDS(
        self, p1: list[dict[Test.MyEnum, str]], p2: list[dict[Test.MyEnum, str]], current: Ice.Current
    ) -> tuple[Sequence[Mapping[Test.MyEnum, str]], Sequence[Mapping[Test.MyEnum, str]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opMyStructMyEnumDS(
        self,
        p1: list[dict[Test.MyStruct, Test.MyEnum]],
        p2: list[dict[Test.MyStruct, Test.MyEnum]],
        current: Ice.Current,
    ) -> tuple[Sequence[Mapping[Test.MyStruct, Test.MyEnum]], Sequence[Mapping[Test.MyStruct, Test.MyEnum]]]:
        p3 = list(p2)
        p3.extend(p1)
        r = p1[::-1]
        return (r, p3)

    @override
    def opByteByteSD(
        self, p1: dict[int, bytes], p2: dict[int, bytes], current: Ice.Current
    ) -> tuple[Mapping[int, bytes], Mapping[int, bytes]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opBoolBoolSD(
        self, p1: dict[bool, list[bool]], p2: dict[bool, list[bool]], current: Ice.Current
    ) -> tuple[Mapping[bool, list[bool]], Mapping[bool, list[bool]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opShortShortSD(
        self, p1: dict[int, list[int]], p2: dict[int, list[int]], current: Ice.Current
    ) -> tuple[Mapping[int, list[int]], Mapping[int, list[int]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opIntIntSD(
        self, p1: dict[int, list[int]], p2: dict[int, list[int]], current: Ice.Current
    ) -> tuple[Mapping[int, list[int]], Mapping[int, list[int]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opLongLongSD(
        self, p1: dict[int, list[int]], p2: dict[int, list[int]], current: Ice.Current
    ) -> tuple[Mapping[int, list[int]], Mapping[int, list[int]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opStringFloatSD(
        self, p1: dict[str, list[float]], p2: dict[str, list[float]], current: Ice.Current
    ) -> tuple[Mapping[str, list[float]], Mapping[str, list[float]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opStringDoubleSD(
        self, p1: dict[str, list[float]], p2: dict[str, list[float]], current: Ice.Current
    ) -> tuple[Mapping[str, list[float]], Mapping[str, list[float]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opStringStringSD(
        self, p1: dict[str, list[str]], p2: dict[str, list[str]], current: Ice.Current
    ) -> tuple[Mapping[str, list[str]], Mapping[str, list[str]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opMyEnumMyEnumSD(
        self, p1: dict[Test.MyEnum, list[Test.MyEnum]], p2: dict[Test.MyEnum, list[Test.MyEnum]], current: Ice.Current
    ) -> tuple[Mapping[Test.MyEnum, list[Test.MyEnum]], Mapping[Test.MyEnum, list[Test.MyEnum]]]:
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    @override
    def opIntS(self, s: list[int], current: Ice.Current) -> Sequence[int]:
        return [-x for x in s]

    @override
    def opByteSOneway(self, s: bytes, current: Ice.Current) -> None:
        with self.lock:
            self.opByteSOnewayCount += 1

    @override
    def opByteSOnewayCallCount(self, current: Ice.Current) -> int:
        with self.lock:
            count = self.opByteSOnewayCount
            self.opByteSOnewayCount = 0
        return count

    @override
    def opContext(self, current: Ice.Current) -> Mapping[str, str]:
        return current.ctx

    @override
    def opDoubleMarshaling(self, p1: float, p2: list[float], current: Ice.Current) -> None:
        d = 1278312346.0 / 13.0
        test(p1 == d)
        for i in p2:
            test(i == d)

    @override
    def opIdempotent(self, current: Ice.Current) -> None:
        test(current.mode == Ice.OperationMode.Idempotent)

    @override
    def opDerived(self, current: Ice.Current) -> None:
        return

    @override
    def opByte1(self, opByte1: int, current: Ice.Current) -> int:
        return opByte1

    @override
    def opShort1(self, opShort1: int, current: Ice.Current) -> int:
        return opShort1

    @override
    def opInt1(self, opInt1: int, current: Ice.Current) -> int:
        return opInt1

    @override
    def opLong1(self, opLong1: int, current: Ice.Current) -> int:
        return opLong1

    @override
    def opFloat1(self, opFloat1: float, current: Ice.Current) -> float:
        return opFloat1

    @override
    def opDouble1(self, opDouble1: float, current: Ice.Current) -> float:
        return opDouble1

    @override
    def opString1(self, opString1: str, current: Ice.Current) -> str:
        return opString1

    @override
    def opStringS1(self, opStringS1: list[str], current: Ice.Current) -> Sequence[str]:
        return opStringS1

    @override
    def opByteBoolD1(self, opByteBoolD1: dict[int, bool], current: Ice.Current) -> Mapping[int, bool]:
        return opByteBoolD1

    @override
    def opStringS2(self, stringS: list[str], current: Ice.Current) -> Sequence[str]:
        return stringS

    @override
    def opByteBoolD2(self, byteBoolD: dict[int, bool], current: Ice.Current) -> Mapping[int, bool]:
        return byteBoolD

    @override
    def opMyClass1(self, opMyClass1: Test.MyClass1 | None, current: Ice.Current) -> Test.MyClass1 | None:
        return opMyClass1

    @override
    def opMyStruct1(self, opMyStruct1: Test.MyStruct1, current: Ice.Current) -> Test.MyStruct1:
        return opMyStruct1

    @override
    def opStringLiterals(self, current: Ice.Current) -> Sequence[str]:
        return [
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

    @override
    def opWStringLiterals(self, current: Ice.Current) -> Sequence[str]:
        return self.opStringLiterals(current)

    def opMStruct1(self, current: Ice.Current):
        return Test.Structure()

    @override
    def opMStruct2(self, p1: Test.Structure, current: Ice.Current):
        return (p1, p1)

    @override
    def opMSeq1(self, current: Ice.Current):
        return ()

    @override
    def opMSeq2(self, p1: list[str], current: Ice.Current) -> tuple[Sequence[str], Sequence[str]]:
        return (p1, p1)

    @override
    def opMDict1(self, current: Ice.Current):
        return {}

    @override
    def opMDict2(self, p1: dict[str, str], current: Ice.Current):
        return (p1, p1)
