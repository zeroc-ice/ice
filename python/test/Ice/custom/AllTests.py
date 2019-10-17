#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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

    sys.stdout.write("testing python:array.array... ")
    sys.stdout.flush()

    v = [True, False, True, False, True]
    v1, v2 = custom.opBoolSeq(array.array("b", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == len(v))
    test(len(v2) == len(v))
    for i in range(len(v)):
        test(v1[i] == v[i])
        test(v2[i] == v[i])

    v = []
    v1, v2 = custom.opBoolSeq(array.array("b", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == 0)
    test(len(v2) == 0)

    v = [0, 2, 4, 8, 16, 32, 64, 127]
    v1, v2 = custom.opByteSeq(array.array("b", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == len(v))
    test(len(v2) == len(v))
    for i in range(len(v)):
        test(v1[i] == v[i])
        test(v2[i] == v[i])

    v = []
    v1, v2 = custom.opByteSeq(array.array("b", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == 0)
    test(len(v2) == 0)

    v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
    v1, v2 = custom.opShortSeq(array.array("h", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == len(v))
    test(len(v2) == len(v))
    for i in range(len(v)):
        test(v1[i] == v[i])
        test(v2[i] == v[i])

    v = []
    v1, v2 = custom.opShortSeq(array.array("h", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == 0)
    test(len(v2) == 0)

    v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
    v1, v2 = custom.opIntSeq(array.array("i", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == len(v))
    test(len(v2) == len(v))
    for i in range(len(v)):
        test(v1[i] == v[i])
        test(v2[i] == v[i])

    v = []
    v1, v2 = custom.opIntSeq(array.array("i", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == 0)
    test(len(v2) == 0)

    #
    # The array "q" type specifier is new in Python 3.3
    #
    if sys.version_info[:2] >= (3, 3):
        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1, v2 = custom.opLongSeq(array.array("q", v))
        test(isinstance(v1, array.array))
        test(isinstance(v2, array.array))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(v1[i] == v[i])
            test(v2[i] == v[i])

        v = []
        v1, v2 = custom.opLongSeq(array.array("q", v))
        test(isinstance(v1, array.array))
        test(isinstance(v2, array.array))
        test(len(v1) == 0)
        test(len(v2) == 0)

    v = [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256]
    v1, v2 = custom.opFloatSeq(array.array("f", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == len(v))
    test(len(v2) == len(v))
    for i in range(len(v)):
        test(round(v1[i], 1) == round(v[i], 1))
        test(round(v2[i], 1) == round(v[i], 1))

    v = []
    v1, v2 = custom.opFloatSeq(array.array("f", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == 0)
    test(len(v2) == 0)

    v = [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256]
    v1, v2 = custom.opDoubleSeq(array.array("d", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == len(v))
    test(len(v2) == len(v))
    for i in range(len(v)):
        test(round(v1[i], 1) == round(v[i], 1))
        test(round(v2[i], 1) == round(v[i], 1))

    v = []
    v1, v2 = custom.opDoubleSeq(array.array("d", v))
    test(isinstance(v1, array.array))
    test(isinstance(v2, array.array))
    test(len(v1) == 0)
    test(len(v2) == 0)


    d = Test.D()
    d.boolSeq = array.array("b", [True, False, True, False, True])
    d.byteSeq = array.array("b", [0, 2, 4, 8, 16, 32, 64, 127])
    d.shortSeq = array.array("h", [0, 2, 4, 8, 16, 32, 64, 128, 256])
    d.intSeq = array.array("i", [0, 2, 4, 8, 16, 32, 64, 128, 256])
    #
    # The array "q" type specifier is new in Python 3.3
    #
    if sys.version_info[:2] >= (3, 3):
        d.longSeq = array.array("q", [0, 2, 4, 8, 16, 32, 64, 128, 256])
    d.floatSeq = array.array("f", [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256])
    d.doubleSeq = array.array("d", [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256])

    d1 = custom.opD(d)
    test(isinstance(d1.boolSeq, array.array))
    test(len(d1.boolSeq) == len(d.boolSeq))
    for i in range(len(d.boolSeq)):
        test(d.boolSeq[i] == d1.boolSeq[i])

    test(isinstance(d1.byteSeq, array.array))
    test(len(d1.byteSeq) == len(d.byteSeq))
    for i in range(len(d.byteSeq)):
        test(d.byteSeq[i] == d1.byteSeq[i])

    test(isinstance(d1.intSeq, array.array))
    test(len(d1.intSeq) == len(d.intSeq))
    for i in range(len(d.intSeq)):
        test(d.intSeq[i] == d1.intSeq[i])

    #
    # The array "q" type specifier is new in Python 3.3
    #
    if sys.version_info[:2] >= (3, 3):
        test(isinstance(d1.longSeq, array.array))
        test(len(d1.longSeq) == len(d.longSeq))
        for i in range(len(d.longSeq)):
            test(d.longSeq[i] == d1.longSeq[i])

    test(isinstance(d1.floatSeq, array.array))
    test(len(d1.floatSeq) == len(d.floatSeq))
    for i in range(len(d.floatSeq)):
        test(round(d.floatSeq[i], 1) == round(d1.floatSeq[i], 1))

    test(isinstance(d1.doubleSeq, array.array))
    test(len(d1.doubleSeq) == len(d.doubleSeq))
    for i in range(len(d.doubleSeq)):
        test(round(d.doubleSeq[i], 1) == round(d1.doubleSeq[i], 1))

    d1 = custom.opD(Test.D())
    test(d1.boolSeq == Ice.Unset)
    test(d1.byteSeq == Ice.Unset)
    test(d1.intSeq == Ice.Unset)
    test(d1.longSeq == Ice.Unset)
    test(d1.floatSeq == Ice.Unset)
    test(d1.doubleSeq == Ice.Unset)
    #
    # With python 3.3 we use the new buffer interface for marshaling
    # sequences of types that implement the buffer protocol and this
    # allow Ice to check that the container item size and endianness
    #
    if sys.version_info[0] >= 3:
        try:
            custom.opBoolSeq(array.array("h", [1, 2, 3, 4]))
            test(False)
        except ValueError:
            pass

        try:
            custom.opShortSeq(array.array("i", [1, 2, 3, 4]))
            test(False)
        except ValueError:
            pass

        try:
            custom.opIntSeq(array.array("h", [1, 2, 3, 4]))
            test(False)
        except ValueError:
            pass

        try:
            custom.opLongSeq(array.array("h", [1, 2, 3, 4]))
            test(False)
        except ValueError:
            pass

        try:
            custom.opFloatSeq(array.array("h", [1, 2, 3, 4]))
            test(False)
        except ValueError:
            pass

        try:
            custom.opDoubleSeq(array.array("h", [1, 2, 3, 4]))
            test(False)
        except ValueError:
            pass

    try:
        custom.opBogusArrayNotExistsFactory()
        test(False)
    except ImportError:
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

    try:
        import numpy
        ref = "test.numpy:{0}".format(helper.getTestEndpoint())
        base = communicator.stringToProxy(ref)
        test(base)

        custom = Test.NumPy.CustomPrx.checkedCast(base)
        test(custom)
        sys.stdout.write("testing python:numpy.ndarray... ")
        sys.stdout.flush()

        v = [True, False, True, False, True]
        v1, v2 = custom.opBoolSeq(numpy.array(v, numpy.bool_))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(v1[i] == v[i])
            test(v2[i] == v[i])

        v = []
        v1, v2 = custom.opBoolSeq(numpy.array(v, numpy.bool_))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        v = [0, 2, 4, 8, 16, 32, 64, 127]
        v1, v2 = custom.opByteSeq(numpy.array(v, numpy.int8))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(v1[i] == v[i])
            test(v2[i] == v[i])

        v = []
        v1, v2 = custom.opByteSeq(numpy.array(v, numpy.int8))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1, v2 = custom.opShortSeq(numpy.array(v, numpy.int16))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(v1[i] == v[i])
            test(v2[i] == v[i])

        v = []
        v1, v2 = custom.opShortSeq(numpy.array(v, numpy.int16))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1, v2 = custom.opIntSeq(numpy.array(v, numpy.int32))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(v1[i] == v[i])
            test(v2[i] == v[i])

        v = []
        v1, v2 = custom.opIntSeq(numpy.array(v, numpy.int32))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        v = [0, 2, 4, 8, 16, 32, 64, 128, 256]
        v1, v2 = custom.opLongSeq(numpy.array(v, numpy.int64))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(v1[i] == v[i])
            test(v2[i] == v[i])

        v = []
        v1, v2 = custom.opLongSeq(numpy.array(v, numpy.int64))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        v = [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256]
        v1, v2 = custom.opFloatSeq(numpy.array(v, numpy.float32))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(round(float(v1[i]), 1) == round(v[i], 1))
            test(round(float(v2[i]), 1) == round(v[i], 1))

        v = []
        v1, v2 = custom.opFloatSeq(numpy.array(v, numpy.float32))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        v = [0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256]
        v1, v2 = custom.opDoubleSeq(numpy.array(v, numpy.float64))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v))
        test(len(v2) == len(v))
        for i in range(len(v)):
            test(round(float(v1[i]), 1) == round(v[i], 1))
            test(round(float(v2[i]), 1) == round(v[i], 1))

        v = []
        v1, v2 = custom.opDoubleSeq(numpy.array(v, numpy.float64))
        test(isinstance(v1, numpy.ndarray))
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == 0)
        test(len(v2) == 0)

        d = Test.NumPy.D()
        d.boolSeq = numpy.array([True, False, True, False, True], numpy.bool_)
        d.byteSeq = numpy.array([0, 2, 4, 8, 16, 32, 64, 127], numpy.int8)
        d.shortSeq = numpy.array([0, 2, 4, 8, 16, 32, 64, 128, 256], numpy.int16)
        d.intSeq = numpy.array([0, 2, 4, 8, 16, 32, 64, 128, 256], numpy.int32)
        d.longSeq = numpy.array([0, 2, 4, 8, 16, 32, 64, 128, 256], numpy.int64)
        d.floatSeq = numpy.array([0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256], numpy.float32)
        d.doubleSeq = numpy.array([0.1, 0.2, 0.4, 0.8, 0.16, 0.32, 0.64, 0.128, 0.256], numpy.float64)

        d1 = custom.opD(d)
        test(isinstance(d1.boolSeq, numpy.ndarray))
        test(len(d1.boolSeq) == len(d.boolSeq))
        for i in range(len(d.boolSeq)):
            test(d.boolSeq[i] == d1.boolSeq[i])

        test(isinstance(d1.byteSeq, numpy.ndarray))
        test(len(d1.byteSeq) == len(d.byteSeq))
        for i in range(len(d.byteSeq)):
            test(d.byteSeq[i] == d1.byteSeq[i])

        test(isinstance(d1.intSeq, numpy.ndarray))
        test(len(d1.intSeq) == len(d.intSeq))
        for i in range(len(d.intSeq)):
            test(d.intSeq[i] == d1.intSeq[i])

        test(isinstance(d1.longSeq, numpy.ndarray))
        test(len(d1.longSeq) == len(d.longSeq))
        for i in range(len(d.longSeq)):
            test(d.longSeq[i] == d1.longSeq[i])

        test(isinstance(d1.floatSeq, numpy.ndarray))
        test(len(d1.floatSeq) == len(d.floatSeq))
        for i in range(len(d.floatSeq)):
            test(round(d.floatSeq[i], 1) == round(d1.floatSeq[i], 1))

        test(isinstance(d1.doubleSeq, numpy.ndarray))
        test(len(d1.doubleSeq) == len(d.doubleSeq))
        for i in range(len(d.doubleSeq)):
            test(round(d.doubleSeq[i], 1) == round(d1.doubleSeq[i], 1))

        d1 = custom.opD(Test.NumPy.D())
        test(d1.boolSeq == Ice.Unset)
        test(d1.byteSeq == Ice.Unset)
        test(d1.intSeq == Ice.Unset)
        test(d1.longSeq == Ice.Unset)
        test(d1.floatSeq == Ice.Unset)
        test(d1.doubleSeq == Ice.Unset)

        v1 = numpy.array([numpy.complex128(1 + 1j),
                          numpy.complex128(2 + 2j),
                          numpy.complex128(3 + 3j),
                          numpy.complex128(4 + 4j)], numpy.complex128)
        v2 = custom.opComplex128Seq(v1)
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v2))
        for i in range(len(v1)):
            test(v1[i] == v2[i])

        v1 = numpy.array([], numpy.complex128)
        v2 = custom.opComplex128Seq(v1)
        test(isinstance(v2, numpy.ndarray))
        test(len(v1) == len(v2))

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
            custom.opBogusNumpyArrayType()
            test(False)
        except ValueError:
            pass

        print("ok")
    except ImportError:
        print("numpy not installed skiping python:numpy.ndarray testing")
        pass

    return custom
