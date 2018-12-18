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


try:
    import numpy
    hasNumPy = True
except ImportError:
    hasNumPy = False
    pass

#
# Use separate try/except to ensure loadSlice correctly report ImportError
# in ausence of numpy.
#
try:
    TestHelper.loadSlice("TestNumPy.ice")
except ImportError:
    pass

import sys
import Test
import Ice
import array

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CustomI(Test.Custom):
    def opByteString1(self, b1, current=None):
        if sys.version_info[0] == 2:
            test(isinstance(b1, str))
        else:
            test(isinstance(b1, bytes))
        return (b1, b1)

    def opByteString2(self, b1, current=None):
        test(isinstance(b1, list))
        return (b1, b1)

    def opByteList1(self, b1, current=None):
        test(isinstance(b1, list))
        return (b1, b1)

    def opByteList2(self, b1, current=None):
        test(isinstance(b1, tuple))
        return (b1, b1)

    def opStringList1(self, s1, current=None):
        test(isinstance(s1, list))
        return (s1, s1)

    def opStringList2(self, s1, current=None):
        test(isinstance(s1, tuple))
        return (s1, s1)

    def opStringTuple1(self, s1, current=None):
        test(isinstance(s1, tuple))
        return (s1, s1)

    def opStringTuple2(self, s1, current=None):
        test(isinstance(s1, list))
        return (s1, s1)

    def sendS(self, val, current=None):
        if sys.version_info[0] == 2:
            test(isinstance(val.b1, str))
        else:
            test(isinstance(val.b1, bytes))
        test(isinstance(val.b2, list))
        if sys.version_info[0] == 2:
            test(isinstance(val.b3, str))
        else:
            test(isinstance(val.b3, bytes))
        test(isinstance(val.b4, list))
        test(isinstance(val.s1, list))
        test(isinstance(val.s2, tuple))
        test(isinstance(val.s3, tuple))
        test(isinstance(val.s4, list))

    def sendC(self, val, current=None):
        if sys.version_info[0] == 2:
            test(isinstance(val.b1, str))
        else:
            test(isinstance(val.b1, bytes))
        test(isinstance(val.b2, list))
        if sys.version_info[0] == 2:
            test(isinstance(val.b3, str))
        else:
            test(isinstance(val.b3, bytes))
        test(isinstance(val.b4, list))
        test(isinstance(val.s1, list))
        test(isinstance(val.s2, tuple))
        test(isinstance(val.s3, tuple))
        test(isinstance(val.s4, list))

    def opBoolSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opByteSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opShortSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opIntSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opLongSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opFloatSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opDoubleSeq(self, v1, current):
        test(isinstance(v1, array.array))
        return v1, v1

    def opBogusArrayNotExistsFactory(self, current):
        return [True, False, True, False]

    def opBogusArrayThrowFactory(self, current):
        return [True, False, True, False]

    def opBogusArrayType(self, current):
        return [True, False, True, False]

    def opBogusArrayNoneFactory(self, current):
        return [True, False, True, False]

    def opBogusArraySignatureFactory(self, current):
        return [True, False, True, False]

    def opBogusArrayNoCallableFactory(self, current):
        return [True, False, True, False]

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

if hasNumPy:

    class NumPyCustomI(Test.NumPy.Custom):

        def opBoolSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opByteSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opShortSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opIntSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opLongSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opFloatSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opDoubleSeq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1, v1

        def opComplex128Seq(self, v1, current):
            test(isinstance(v1, numpy.ndarray))
            return v1

        def opBoolMatrix(self, current):
            return numpy.array([[True, False, True],
                                [True, False, True],
                                [True, False, True]], numpy.bool_)

        def opByteMatrix(self, current):
            return numpy.array([[1, 0, 1],
                                [1, 0, 1],
                                [1, 0, 1]], numpy.int8)

        def opShortMatrix(self, current):
            return numpy.array([[1, 0, 1],
                                [1, 0, 1],
                                [1, 0, 1]], numpy.int16)

        def opIntMatrix(self, current):
            return numpy.array([[1, 0, 1],
                                [1, 0, 1],
                                [1, 0, 1]], numpy.int32)

        def opLongMatrix(self, current):
            return numpy.array([[1, 0, 1],
                                [1, 0, 1],
                                [1, 0, 1]], numpy.int64)

        def opFloatMatrix(self, current):
            return numpy.array([[1.1, 0.1, 1.1],
                                [1.1, 0.1, 1.1],
                                [1.1, 0.1, 1.1]], numpy.float32)

        def opDoubleMatrix(self, current):
            return numpy.array([[1.1, 0.1, 1.1],
                                [1.1, 0.1, 1.1],
                                [1.1, 0.1, 1.1]], numpy.float64)

        def opBogusNumpyArrayType(self, current):
            return [True, False, True, False]

        def shutdown(self, current=None):
            current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(CustomI(), Ice.stringToIdentity("test"))
            if hasNumPy:
                adapter.add(NumPyCustomI(), Ice.stringToIdentity("test.numpy"))
            adapter.activate()
            communicator.waitForShutdown()
