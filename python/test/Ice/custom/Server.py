#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

try:
    import numpy

    hasNumPy = True
except ImportError:
    hasNumPy = False
    pass

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from generated.test.Ice.custom import Test

if hasNumPy:
    if "--load-slice" in sys.argv:
        TestHelper.loadSlice("TestNumPy.ice")

    from generated.test.Ice.custom.Test import NumPy
import array

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class CustomI(Test.Custom):
    def opByteString1(self, b1, current: Ice.Current):
        test(isinstance(b1, bytes))
        return (b1, b1)

    def opByteString2(self, b1, current: Ice.Current):
        test(isinstance(b1, list))
        return (b1, b1)

    def opByteList1(self, b1, current: Ice.Current):
        test(isinstance(b1, list))
        return (b1, b1)

    def opByteList2(self, b1, current: Ice.Current):
        test(isinstance(b1, tuple))
        return (b1, b1)

    def opStringList1(self, s1, current: Ice.Current):
        test(isinstance(s1, list))
        return (s1, s1)

    def opStringList2(self, s1, current: Ice.Current):
        test(isinstance(s1, tuple))
        return (s1, s1)

    def opStringTuple1(self, s1, current: Ice.Current):
        test(isinstance(s1, tuple))
        return (s1, s1)

    def opStringTuple2(self, s1, current: Ice.Current):
        test(isinstance(s1, list))
        return (s1, s1)

    def sendS(self, val, current: Ice.Current):
        test(isinstance(val.b1, bytes))
        test(isinstance(val.b2, list))
        test(isinstance(val.b4, list))
        test(isinstance(val.s1, list))
        test(isinstance(val.s2, tuple))
        test(isinstance(val.s3, tuple))

    def sendC(self, val, current: Ice.Current):
        test(isinstance(val.b1, bytes))
        test(isinstance(val.b2, list))
        test(isinstance(val.b4, list))
        test(isinstance(val.s1, list))
        test(isinstance(val.s2, tuple))
        test(isinstance(val.s3, tuple))

    def opBoolSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opByteSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opShortSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opIntSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opLongSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opFloatSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opDoubleSeq(self, v1, current: Ice.Current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opBogusArrayNotExistsFactory(self, current: Ice.Current):
        return [True, False, True, False]

    def opBogusArrayThrowFactory(self, current: Ice.Current):
        return [True, False, True, False]

    def opBogusArrayType(self, current: Ice.Current):
        return [True, False, True, False]

    def opBogusArrayNoneFactory(self, current: Ice.Current):
        return [True, False, True, False]

    def opBogusArraySignatureFactory(self, current: Ice.Current):
        return [True, False, True, False]

    def opBogusArrayNoCallableFactory(self, current: Ice.Current):
        return [True, False, True, False]

    def opD(self, d, current: Ice.Current):
        return d

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


if hasNumPy:

    class NumPyCustomI(NumPy.Custom):
        def opBoolSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opByteSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opShortSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opIntSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opLongSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opFloatSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opDoubleSeq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opComplex128Seq(self, v1, current: Ice.Current):
            test(isinstance(v1, numpy.ndarray))
            return v1

        def opBoolMatrix(self, current: Ice.Current):
            return numpy.array(
                [[True, False, True], [True, False, True], [True, False, True]],
                numpy.bool_,
            )

        def opByteMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int8)

        def opShortMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int16)

        def opIntMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int32)

        def opLongMatrix(self, current: Ice.Current):
            return numpy.array([[1, 0, 1], [1, 0, 1], [1, 0, 1]], numpy.int64)

        def opFloatMatrix(self, current: Ice.Current):
            return numpy.array([[1.1, 0.1, 1.1], [1.1, 0.1, 1.1], [1.1, 0.1, 1.1]], numpy.float32)

        def opDoubleMatrix(self, current: Ice.Current):
            return numpy.array([[1.1, 0.1, 1.1], [1.1, 0.1, 1.1], [1.1, 0.1, 1.1]], numpy.float64)

        def opBogusNumpyArrayType(self, current: Ice.Current):
            return [True, False, True, False]

        def opD(self, d, current: Ice.Current):
            return d

        def shutdown(self, current: Ice.Current):
            current.adapter.getCommunicator().shutdown()


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
