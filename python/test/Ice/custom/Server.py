#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from collections.abc import Sequence
from typing import override

from TestHelper import TestHelper, test

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import array

from generated.test.Ice.custom import Test

import Ice


class CustomI(Test.Custom):
    @override
    def opByteString1(self, b1: bytes, current: Ice.Current) -> tuple[bytes, bytes]:
        test(isinstance(b1, bytes))
        return (b1, b1)

    @override
    def opByteString2(self, b1: list[int], current: Ice.Current):
        test(isinstance(b1, list))
        return (b1, b1)

    @override
    def opByteList1(self, b1: list[int], current: Ice.Current) -> tuple[Sequence[int], Sequence[int]]:
        test(isinstance(b1, list))
        return (b1, b1)

    @override
    def opByteList2(self, b1: tuple[int, ...], current: Ice.Current) -> tuple[Sequence[int], tuple[int, ...]]:
        test(isinstance(b1, tuple))
        return (b1, b1)

    @override
    def opStringList1(self, s1: list[str], current: Ice.Current) -> tuple[Sequence[str], Sequence[str]]:
        test(isinstance(s1, list))
        return (s1, s1)

    @override
    def opStringList2(self, s1: tuple[str, ...], current: Ice.Current) -> tuple[tuple[str, ...], tuple[str, ...]]:
        test(isinstance(s1, tuple))
        return (s1, s1)

    @override
    def opStringTuple1(self, s1: tuple[str, ...], current: Ice.Current) -> tuple[tuple[str, ...], tuple[str, ...]]:
        test(isinstance(s1, tuple))
        return (s1, s1)

    @override
    def opStringTuple2(self, s1: list[str], current: Ice.Current) -> tuple[Sequence[str], Sequence[str]]:
        test(isinstance(s1, list))
        return (s1, s1)

    @override
    def sendS(self, val: Test.S, current: Ice.Current):
        test(isinstance(val.b1, bytes))
        test(isinstance(val.b2, list))
        test(isinstance(val.b4, list))
        test(isinstance(val.s1, list))
        test(isinstance(val.s2, tuple))
        test(isinstance(val.s3, tuple))

    @override
    def sendC(self, val: Test.C | None, current: Ice.Current):
        assert val is not None
        test(isinstance(val.b1, bytes))
        test(isinstance(val.b2, list))
        test(isinstance(val.b4, list))
        test(isinstance(val.s1, list))
        test(isinstance(val.s2, tuple))
        test(isinstance(val.s3, tuple))

    @override
    def opBoolSeq(self, v1: array.array[int], current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opByteSeq(self, v1: array.array[int], current: Ice.Current) -> tuple[Sequence[int], Sequence[int]]:
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opShortSeq(self, v1: array.array[int], current: Ice.Current) -> tuple[Sequence[int], Sequence[int]]:
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opIntSeq(self, v1: array.array[int], current: Ice.Current) -> tuple[Sequence[int], Sequence[int]]:
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opLongSeq(self, v1: array.array[int], current: Ice.Current) -> tuple[Sequence[int], Sequence[int]]:
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opFloatSeq(self, v1: array.array[float], current: Ice.Current) -> tuple[Sequence[float], Sequence[float]]:
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opDoubleSeq(self, v1: array.array[float], current: Ice.Current) -> tuple[Sequence[float], Sequence[float]]:
        test(isinstance(v1, array.array))
        return v1, v1

    @override
    def opBogusArrayNotExistsFactory(self, current: Ice.Current):
        return [True, False, True, False]

    @override
    def opBogusArrayThrowFactory(self, current: Ice.Current):
        return [True, False, True, False]

    @override
    def opBogusArrayType(self, current: Ice.Current):
        return [True, False, True, False]

    @override
    def opBogusArrayNoneFactory(self, current: Ice.Current):
        return [True, False, True, False]

    @override
    def opBogusArraySignatureFactory(self, current: Ice.Current):
        return [True, False, True, False]

    @override
    def opBogusArrayNoCallableFactory(self, current: Ice.Current):
        return [True, False, True, False]

    def opD(self, d: Test.D | None, current: Ice.Current) -> Test.D | None:
        return d

    @override
    def opM(self, m: Test.M, current: Ice.Current) -> Test.M:
        return m

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


hasNumPy = False
try:
    import numpy

    hasNumPy = True

    if "--load-slice" in sys.argv:
        TestHelper.loadSlice("TestNumPy.ice")

    from generated.test.Ice.custom.Test import NumPy

    class NumPyCustomI(NumPy.Custom):
        @override
        def opBoolSeq(self, v1: numpy.typing.NDArray[numpy.bool], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opByteSeq(self, v1: numpy.typing.NDArray[numpy.int8], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opShortSeq(self, v1: numpy.typing.NDArray[numpy.int16], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opIntSeq(self, v1: numpy.typing.NDArray[numpy.int32], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opLongSeq(self, v1: numpy.typing.NDArray[numpy.int64], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opFloatSeq(self, v1: numpy.typing.NDArray[numpy.float32], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opDoubleSeq(self, v1: numpy.typing.NDArray[numpy.float64], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        @override
        def opComplex128Seq(self, v1: numpy.typing.NDArray[numpy.complex128], current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1

        @override
        def opBoolMatrix(self, current: Ice.Current):
            return numpy.array(
                [[True, False, True], [True, False, True], [True, False, True]],
                numpy.bool_,
            )

        @override
        def opByteMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int8)

        @override
        def opShortMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int16)

        @override
        def opIntMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int32)

        @override
        def opLongMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int64)

        @override
        def opFloatMatrix(self, current: Ice.Current):
            return numpy.array([[1.1, 0.1, 1.1], [1.1, 0.1, 1.1], [1.1, 0.1, 1.1]], numpy.float32)

        @override
        def opDoubleMatrix(self, current: Ice.Current):
            return numpy.array([[1.1, 0.1, 1.1], [1.1, 0.1, 1.1], [1.1, 0.1, 1.1]], numpy.float64)

        @override
        def opBogusNumpyArrayType(self, current: Ice.Current):
            return [True, False, True, False]

        @override
        def opD(self, d: NumPy.D | None, current: Ice.Current) -> NumPy.D | None:
            return d

        @override
        def shutdown(self, current: Ice.Current):
            current.adapter.getCommunicator().shutdown()

except ImportError:
    pass


class Server(TestHelper):
    def run(self, args: list[str]):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(CustomI(), Ice.stringToIdentity("test"))
            if hasNumPy:
                adapter.add(NumPyCustomI(), Ice.stringToIdentity("test.numpy"))
            adapter.activate()
            communicator.waitForShutdown()
