# Copyright (c) ZeroC, Inc.

import array

from .Builtin import BuiltinBool, BuiltinByte, BuiltinDouble, BuiltinFloat, BuiltinInt, BuiltinLong, BuiltinShort

BuiltinTypes = [
    BuiltinBool,
    BuiltinByte,
    BuiltinShort,
    BuiltinInt,
    BuiltinLong,
    BuiltinFloat,
    BuiltinDouble,
]
BuiltinArrayTypes = ["b", "b", "h", "i", "q", "f", "d"]


def createArray(view: memoryview | None, t: int) -> array.array:
    if t not in BuiltinTypes:
        raise ValueError(f"'{t}' is not an array builtin type")

    a = array.array(BuiltinArrayTypes[t])
    if view is not None:
        a.frombytes(view)
    return a


try:
    import numpy

    BuiltinNumpyTypes = [
        numpy.bool,
        numpy.int8,
        numpy.int16,
        numpy.int32,
        numpy.int64,
        numpy.float32,
        numpy.float64,
    ]

    def createNumPyArray(view: memoryview | None, t: int) -> numpy.ndarray:
        if t not in BuiltinTypes:
            raise ValueError(f"'{t}' is not an array builtin type")
        if view is not None:
            return numpy.frombuffer(view.tobytes(), BuiltinNumpyTypes[t])
        else:
            return numpy.empty(0, BuiltinNumpyTypes[t])

except ImportError:
    pass
