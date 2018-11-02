# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice


def myBoolSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myByteSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myShortSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myIntSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myLongSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myFloatSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myDoubleSeq(buffer, type, copy):
    return Ice.createNumPyArray(buffer, type, copy)


def myComplex128Seq(buffer, type, copy):
    import numpy
    return numpy.frombuffer(buffer.tobytes() if copy else buffer, numpy.complex128)


def myMatrix3x3(buffer, type, copy):
    a = Ice.createNumPyArray(buffer, type, copy)
    a.shape = (3, 3)
    return a


def myBogusArrayThrowFactory(buffer, type, copy):
    raise ValueError()


def myBogusNumpyArrayType(buffer, type, copy):
    return Ice.createNumPyArray(buffer, 1024, copy)


def myBogusArrayType(buffer, type, copy):
    return Ice.createArray(buffer, 1024, copy)


def myBogusArrayNoneFactory(buffer, type, copy):
    return None


def myBogusArraySignatureFactory():
    return []


myNoCallableFactory = 5
