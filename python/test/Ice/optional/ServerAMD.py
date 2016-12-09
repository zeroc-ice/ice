#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
Ice.loadSlice('TestAMD.ice')
import Test

class InitialI(Test.Initial):

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def pingPong(self, o, current=None):
        return Ice.Future.completed(o)

    def opOptionalException(self, a, b, o, current=None):
        f = Ice.Future()
        f.set_exception(Test.OptionalException(False, a, b, o))
        return f

    def opDerivedException(self, a, b, o, current=None):
        f = Ice.Future()
        f.set_exception(Test.DerivedException(False, a, b, o, b, o))
        return f

    def opRequiredException(self, a, b, o, current=None):
        e = Test.RequiredException()
        e.a = a
        e.b = b
        e.o = o
        if b is not Ice.Unset:
            e.ss = b
        if o is not Ice.Unset:
            e.o2 = o
        f = Ice.Future()
        f.set_exception(e)
        return f

    def opByte(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opBool(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opShort(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opInt(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opLong(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opFloat(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opDouble(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opString(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opMyEnum(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opSmallStruct(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opFixedStruct(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opVarStruct(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opOneOptional(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opOneOptionalProxy(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opByteSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opBoolSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opShortSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opIntSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opLongSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opFloatSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opDoubleSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opStringSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opSmallStructSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opSmallStructList(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opFixedStructSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opFixedStructList(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opVarStructSeq(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opSerializable(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opIntIntDict(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opStringIntDict(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opIntOneOptionalDict(self, p1, current=None):
        return Ice.Future.completed((p1, p1))

    def opClassAndUnknownOptional(self, p, current=None):
        return Ice.Future.completed(None)

    def sendOptionalClass(self, req, o, current=None):
        return Ice.Future.completed(None)

    def returnOptionalClass(self, req, current=None):
        return Ice.Future.completed(Test.OneOptional(53))

    def opG(self, g, current=None):
        return Ice.Future.completed(g)

    def opVoid(self, current=None):
        return Ice.Future.completed(None)

    def opMStruct1(self, current):
        return Ice.Future.completed(Test.SmallStruct())

    def opMStruct2(self, p1, current):
        return Ice.Future.completed((p1, p1))

    def opMSeq1(self, current):
        return Ice.Future.completed([])

    def opMSeq2(self, p1, current):
        return Ice.Future.completed((p1, p1))

    def opMDict1(self, current):
        return Ice.Future.completed({})

    def opMDict2(self, p1, current):
        return Ice.Future.completed((p1, p1))

    def opMG1(self, current):
        return Ice.Future.completed(Test.G())

    def opMG2(self, p1, current):
        return Ice.Future.completed((p1, p1))

    def supportsRequiredParams(self, current=None):
        return Ice.Future.completed(False)

    def supportsJavaSerializable(self, current=None):
        return Ice.Future.completed(True)

    def supportsCsharpSerializable(self, current=None):
        return Ice.Future.completed(False)

    def supportsCppStringView(self, current=None):
        return Ice.Future.completed(False)

    def supportsNullOptional(self, current=None):
        return Ice.Future.completed(True)

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    initial = InitialI()
    adapter.add(initial, Ice.stringToIdentity("initial"))
    adapter.activate()

    communicator.waitForShutdown()
    return True

try:
    communicator = Ice.initialize(sys.argv)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
