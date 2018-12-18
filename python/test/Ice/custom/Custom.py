# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice

try:
    import numpy
    hasNumPy = True
except ImportError:
    hasNumPy = False

def myBoolSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)


def myByteSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)


def myShortSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)


def myIntSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)


def myLongSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)


def myFloatSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)


def myDoubleSeq(buffer, type, copy):
    return Ice.createArray(buffer, type, copy)

if hasNumPy:

    def myNumPyBoolSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyByteSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyShortSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyIntSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyLongSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyFloatSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyDoubleSeq(buffer, type, copy):
        return Ice.createNumPyArray(buffer, type, copy)


    def myNumPyComplex128Seq(buffer, type, copy):
        import numpy
        return numpy.frombuffer(buffer.tobytes() if copy else buffer, numpy.complex128)


    def myNumPyMatrix3x3(buffer, type, copy):
        a = Ice.createNumPyArray(buffer, type, copy)
        a.shape = (3, 3)
        return a

    def myBogusNumpyArrayType(buffer, type, copy):
        return Ice.createNumPyArray(buffer, 1024, copy)


def myBogusArrayThrowFactory(buffer, type, copy):
    raise ValueError()


def myBogusArrayType(buffer, type, copy):
    return Ice.createArray(buffer, 1024, copy)


def myBogusArrayNoneFactory(buffer, type, copy):
    return None


def myBogusArraySignatureFactory():
    return []


myNoCallableFactory = 5
