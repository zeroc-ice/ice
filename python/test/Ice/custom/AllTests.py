# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, string, re, traceback, Ice, Test, array


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


def allTests(helper, communicator):
    ref = "test:{0}".format(helper.getTestEndpoint())
    base = communicator.stringToProxy(ref)
    test(base)

    custom = Test.CustomPrx.checkedCast(base)
    test(custom)

    byteList = [1, 2, 3, 4, 5]
    if sys.version_info[0] == 2:
        byteString = ''.join(map(chr, byteList))
    else:
        byteString = bytes(byteList)
    stringList = ['s1', 's2', 's3']

    sys.stdout.write("testing custom sequences... ")
    sys.stdout.flush()

    (r, b2) = custom.opByteString1(byteString)
    if sys.version_info[0] == 2:
        test(isinstance(r, str))
        test(isinstance(b2, str))
    else:
        test(isinstance(r, bytes))
        test(isinstance(b2, bytes))
    test(r == byteString)
    test(b2 == byteString)

    (r, b2) = custom.opByteString2(byteString)
    test(isinstance(r, tuple))
    test(isinstance(b2, list))
    for i in range(0, len(byteList)):
        test(r[i] == byteList[i])
        test(b2[i] == byteList[i])

    (r, b2) = custom.opByteList1(byteList)
    test(isinstance(r, list))
    test(isinstance(b2, list))
    for i in range(0, len(byteList)):
        test(r[i] == byteList[i])
        test(b2[i] == byteList[i])

    (r, b2) = custom.opByteList2(byteList)
    if sys.version_info[0] == 2:
        test(isinstance(r, str))
    else:
        test(isinstance(r, bytes))
    test(isinstance(b2, tuple))
    test(r == byteString)
    for i in range(0, len(byteList)):
        test(b2[i] == byteList[i])

    (r, b2) = custom.opStringList1(stringList)
    test(isinstance(r, list))
    test(isinstance(b2, list))
    test(r == stringList)
    test(b2 == stringList)

    (r, b2) = custom.opStringList2(stringList)
    test(isinstance(r, tuple))
    test(isinstance(b2, tuple))
    for i in range(0, len(stringList)):
        test(r[i] == stringList[i])
        test(b2[i] == stringList[i])

    (r, b2) = custom.opStringTuple1(stringList)
    test(isinstance(r, tuple))
    test(isinstance(b2, tuple))
    for i in range(0, len(stringList)):
        test(r[i] == stringList[i])
        test(b2[i] == stringList[i])

    (r, b2) = custom.opStringTuple2(stringList)
    test(isinstance(r, list))
    test(isinstance(b2, list))
    test(r == stringList)
    test(b2 == stringList)

    s = Test.S()
    s.b1 = byteList;
    s.b2 = byteList;
    s.b3 = byteList;
    s.b4 = byteList;
    s.s1 = stringList;
    s.s2 = stringList;
    s.s3 = stringList;
    s.s4 = stringList;
    custom.sendS(s)

    c = Test.C()
    c.b1 = byteList;
    c.b2 = byteList;
    c.b3 = byteList;
    c.b4 = byteList;
    c.s1 = stringList;
    c.s2 = stringList;
    c.s3 = stringList;
    c.s4 = stringList;
    custom.sendC(c)

    print("ok")

    try:
        import numpy
        sys.stdout.write("testing array.array/numpy.array custom sequences... ")
        sys.stdout.flush()

        v = [True, False, True, False, True]
        v1 = array.array("b", v)
        v2 = numpy.array(v, numpy.bool_)
        v3, v4, v5 = custom.opBoolSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v5, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(v3[i] == v[i])
            test(v4[i] == v[i])
            test(v5[i] == v[i])

        v = [0, 2, 4, 8, 16, 32, 64, 127]
        v1 = array.array("b", v)
        v2 = numpy.array(v, numpy.int8)
        v3, v4, v5 = custom.opByteSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v4, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(v3[i] == v[i])
            test(v4[i] == v[i])
            test(v5[i] == v[i])

        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1 = array.array("h", v)
        v2 = numpy.array(v, numpy.int16)
        v3, v4, v5 = custom.opShortSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v5, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(v3[i] == v[i])
            test(v4[i] == v[i])
            test(v5[i] == v[i])

        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1 = array.array("i", v)
        v2 = numpy.array(v, numpy.int32)
        v3, v4, v5 = custom.opIntSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v4, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(v3[i] == v[i])
            test(v4[i] == v[i])
            test(v5[i] == v[i])

        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1 = array.array("l", v)
        v2 = numpy.array(v, numpy.int64)
        v3, v4, v5 = custom.opLongSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v5, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(v3[i] == v[i])
            test(v4[i] == v[i])
            test(v5[i] == v[i])

        v = [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256]
        v1 = array.array("f", v)
        v2 = numpy.array(v, numpy.float32)
        v3, v4, v5 = custom.opFloatSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v5, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(round(v3[i], 1) == round(v[i], 1))
            test(round(float(v4[i]), 1) == round(v[i], 1))
            test(round(float(v5[i]), 1) == round(v[i], 1))

        v = [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256]
        v1 = array.array("d", v)
        v2 = numpy.array(v, numpy.float64)
        v3, v4, v5 = custom.opDoubleSeq(v1, v2, v2)
        test(isinstance(v3, array.array))
        test(isinstance(v4, numpy.ndarray))
        test(isinstance(v5, numpy.ndarray))
        test(len(v3) == len(v))
        test(len(v4) == len(v))
        test(len(v5) == len(v))
        for i in range(len(v)):
            test(round(v3[i], 1) == round(v[i], 1))
            test(round(float(v4[i]), 1) == round(v[i], 1))
            test(round(float(v5[i]), 1) == round(v[i], 1))

        v1 = numpy.array([numpy.complex128(1 + 1j),
                          numpy.complex128(2 + 2j),
                          numpy.complex128(3 + 3j),
                          numpy.complex128(4 + 4j)], numpy.complex128)
        v2 = custom.opComplex128Seq(v1)
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v2))
        for i in range(len(v1)):
            test(v1[i] == v2[i])

        v1 = custom.opBoolMatrix()
        test(numpy.array_equal(v1, numpy.array([[True, False, True],
                                                [True, False, True],
                                                [True, False, True]], numpy.bool_)))

        v1 = custom.opByteMatrix()
        test(numpy.array_equal(v1, numpy.array([[1, 0, 1],
                                                [1, 0, 1],
                                                [1, 0, 1]], numpy.int8)))

        v1 = custom.opShortMatrix()
        test(numpy.array_equal(v1, numpy.array([[1, 0, 1],
                                                [1, 0, 1],
                                                [1, 0, 1]], numpy.int16)))

        v1 = custom.opIntMatrix()
        test(numpy.array_equal(v1, numpy.array([[1, 0, 1],
                                                [1, 0, 1],
                                                [1, 0, 1]], numpy.int32)))

        v1 = custom.opLongMatrix()
        test(numpy.array_equal(v1, numpy.array([[1, 0, 1],
                                                [1, 0, 1],
                                                [1, 0, 1]], numpy.int64)))

        v1 = custom.opFloatMatrix()
        test(numpy.array_equal(v1, numpy.array([[1.1, 0.1, 1.1],
                                                [1.1, 0.1, 1.1],
                                                [1.1, 0.1, 1.1]], numpy.float32)))

        v1 = custom.opDoubleMatrix()
        test(numpy.array_equal(v1, numpy.array([[1.1, 0.1, 1.1],
                                                [1.1, 0.1, 1.1],
                                                [1.1, 0.1, 1.1]], numpy.float64)))
        try:
            custom.opBogusArrayNotExistsFactory()
            test(False)
        except RuntimeError:
            pass

        try:
            custom.opBogusArrayThrowFactory()
            test(False)
        except ValueError:
            pass

        try:
            custom.opBogusArrayType()
            test(False)
        except ValueError:
            pass

        try:
            custom.opBogusNumpyArrayType()
            test(False)
        except ValueError:
            pass

        try:
            custom.opBogusArrayNoneFactory()
            test(False)
        except ValueError:
            pass

        try:
            custom.opBogusArraySignatureFactory()
            test(False)
        except TypeError:
            pass

        try:
            custom.opBogusArrayNoCallableFactory()
            test(False)
        except RuntimeError:
            pass

        print("ok")
    except ImportError:
        print("numpy not installed skiping array.array/numpy.array testing")
        pass

    return custom
