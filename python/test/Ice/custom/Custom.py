# Copyright (c) ZeroC, Inc.

from importlib.util import find_spec

from ArrayUtil import createArray

hasNumPy = find_spec("numpy") is not None


def myBoolSeq(buffer, type):
    return createArray(buffer, type)


def myByteSeq(buffer, type):
    return createArray(buffer, type)


def myShortSeq(buffer, type):
    return createArray(buffer, type)


def myIntSeq(buffer, type):
    return createArray(buffer, type)


def myLongSeq(buffer, type):
    return createArray(buffer, type)


def myFloatSeq(buffer, type):
    return createArray(buffer, type)


def myDoubleSeq(buffer, type):
    return createArray(buffer, type)


if hasNumPy:
    from ArrayUtil import createNumPyArray

    def myNumPyBoolSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyByteSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyShortSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyIntSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyLongSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyFloatSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyDoubleSeq(buffer, type):
        return createNumPyArray(buffer, type)

    def myNumPyComplex128Seq(buffer, type):
        import numpy

        if buffer is not None:
            return numpy.frombuffer(buffer.tobytes(), numpy.complex128)
        else:
            return numpy.empty(0, numpy.complex128)

    def myNumPyMatrix3x3(buffer, type):
        a = createNumPyArray(buffer, type)
        a.shape = (3, 3)
        return a

    def myBogusNumpyArrayType(buffer, type):
        return createNumPyArray(buffer, 1024)


def myBogusArrayThrowFactory(buffer, type):
    raise ValueError()


def myBogusArrayType(buffer, type):
    return createArray(buffer, 1024)


def myBogusArrayNoneFactory(buffer, type):
    return None


def myBogusArraySignatureFactory():
    return []


myNoCallableFactory = 5
