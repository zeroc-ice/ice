# Copyright (c) ZeroC, Inc.

import array

from ArrayUtil import createArray


def myBoolSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


def myByteSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


def myShortSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


def myIntSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


def myLongSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


def myFloatSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


def myDoubleSeq(buffer: memoryview, type: int) -> array.array:
    return createArray(buffer, type)


try:
    import numpy
    from ArrayUtil import createNumPyArray

    def myNumPyBoolSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyByteSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyShortSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyIntSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyLongSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyFloatSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyDoubleSeq(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, type)

    def myNumPyComplex128Seq(buffer: memoryview | None, type: int) -> numpy.ndarray:
        if buffer is not None:
            return numpy.frombuffer(buffer.tobytes(), numpy.complex128)
        else:
            return numpy.empty(0, numpy.complex128)

    def myNumPyMatrix3x3(buffer: memoryview, type: int) -> numpy.ndarray:
        a = createNumPyArray(buffer, type)
        a.shape = (3, 3)
        return a

    def myBogusNumpyArrayType(buffer: memoryview, type: int) -> numpy.ndarray:
        return createNumPyArray(buffer, 1024)

except ImportError:
    pass


def myBogusArrayThrowFactory(buffer: memoryview, type: int) -> None:
    raise ValueError()


def myBogusArrayType(buffer: memoryview, type: int) -> None:
    return createArray(buffer, 1024)  # pyright: ignore


def myBogusArrayNoneFactory(buffer: memoryview | None, type: int) -> None:
    return None


def myBogusArraySignatureFactory():
    return []


myNoCallableFactory = 5
