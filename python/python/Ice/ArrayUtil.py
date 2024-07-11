# Copyright (c) ZeroC, Inc. All rights reserved.

import array

BuiltinBool = 0
BuiltinByte = 1
BuiltinShort = 2
BuiltinInt = 3
BuiltinLong = 4
BuiltinFloat = 5
BuiltinDouble = 6

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


def createArray(view, t, copy):
    if t not in BuiltinTypes:
        raise ValueError(f"'{t}' is not an array builtin type")
    a = array.array(BuiltinArrayTypes[t])
    a.frombytes(view)
    return a


try:
    import numpy

    BuiltinNumpyTypes = [
        numpy.bool_,
        numpy.int8,
        numpy.int16,
        numpy.int32,
        numpy.int64,
        numpy.float32,
        numpy.float64,
    ]

    def createNumPyArray(view, t, copy):
        if t not in BuiltinTypes:
            raise ValueError(f"'{t}' is not an array builtin type")
        return numpy.frombuffer(view.tobytes() if copy else view, BuiltinNumpyTypes[t])

except ImportError:
    pass
