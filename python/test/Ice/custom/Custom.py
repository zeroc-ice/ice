# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice


def myBoolSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myByteSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myShortSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myIntSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myLongSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myFloatSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myDoubleSeq(buffer, type):
    return Ice.createNumPyArray(buffer, type)


def myComplex128Seq(buffer, type):
    import numpy
    return numpy.frombuffer(buffer, numpy.complex128)


def myMatrix3x3(buffer, type):
    a = Ice.createNumPyArray(buffer, type)
    a.shape = (3, 3)
    return a


def myBogusArrayThrowFactory(buffer, type):
    raise ValueError()


def myBogusNumpyArrayType(buffer, type):
    return Ice.createNumPyArray(buffer, 1024)


def myBogusArrayType(buffer, type):
    return Ice.createArray(buffer, 1024)


def myBogusArrayNoneFactory(buffer, type):
    return None


def myBogusArraySignatureFactory():
    return []


myNoCallableFactory = 5
