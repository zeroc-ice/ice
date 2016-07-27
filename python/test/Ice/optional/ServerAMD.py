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

    def shutdown_async(self, cb, current=None):
        current.adapter.getCommunicator().shutdown()
        cb.ice_response()

    def pingPong_async(self, cb, o, current=None):
        cb.ice_response(o)

    def opOptionalException_async(self, cb, a, b, o, current=None):
        cb.ice_exception(Test.OptionalException(False, a, b, o))

    def opDerivedException_async(self, cb, a, b, o, current=None):
        cb.ice_exception(Test.DerivedException(False, a, b, o, b, o))

    def opRequiredException_async(self, cb, a, b, o, current=None):
        e = Test.RequiredException()
        e.a = a
        e.b = b
        e.o = o
        if b is not Ice.Unset:
            e.ss = b
        if o is not Ice.Unset:
            e.o2 = o
        cb.ice_exception(e)

    def opByte_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opBool_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opShort_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opInt_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opLong_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opFloat_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opDouble_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opString_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opMyEnum_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opSmallStruct_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opFixedStruct_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opVarStruct_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opOneOptional_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opOneOptionalProxy_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opByteSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opBoolSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opShortSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opIntSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opLongSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opFloatSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opDoubleSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opStringSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opSmallStructSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opSmallStructList_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opFixedStructSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opFixedStructList_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opVarStructSeq_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opSerializable_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opIntIntDict_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opStringIntDict_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opIntOneOptionalDict_async(self, cb, p1, current=None):
        cb.ice_response(p1, p1)

    def opClassAndUnknownOptional_async(self, cb, p, current=None):
        cb.ice_response()

    def sendOptionalClass_async(self, cb, req, o, current=None):
        cb.ice_response()

    def returnOptionalClass_async(self, cb, req, current=None):
        cb.ice_response(Test.OneOptional(53))

    def opG_async(self, cb, g, current=None):
        cb.ice_response(g)

    def opVoid_async(self, cb, current=None):
        cb.ice_response()

    def supportsRequiredParams_async(self, cb, current=None):
        cb.ice_response(False)

    def supportsJavaSerializable_async(self, cb, current=None):
        cb.ice_response(True)

    def supportsCsharpSerializable_async(self, cb, current=None):
        cb.ice_response(False)

    def supportsCppStringView_async(self, cb, current=None):
        cb.ice_response(False)

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
