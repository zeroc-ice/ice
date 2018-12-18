#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Ice
import Test


class InitialI(Test.Initial):

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def pingPong(self, o, current=None):
        return o

    def opOptionalException(self, a, b, o, current=None):
        raise Test.OptionalException(False, a, b, o)

    def opDerivedException(self, a, b, o, current=None):
        raise Test.DerivedException(False, a, b, o, b, o)

    def opRequiredException(self, a, b, o, current=None):
        e = Test.RequiredException()
        e.a = a
        e.b = b
        e.o = o
        if b is not Ice.Unset:
            e.ss = b
        if o is not Ice.Unset:
            e.o2 = o
        raise e

    def opByte(self, p1, current=None):
        return (p1, p1)

    def opBool(self, p1, current=None):
        return (p1, p1)

    def opShort(self, p1, current=None):
        return (p1, p1)

    def opInt(self, p1, current=None):
        return (p1, p1)

    def opLong(self, p1, current=None):
        return (p1, p1)

    def opFloat(self, p1, current=None):
        return (p1, p1)

    def opDouble(self, p1, current=None):
        return (p1, p1)

    def opString(self, p1, current=None):
        return (p1, p1)

    def opMyEnum(self, p1, current=None):
        return (p1, p1)

    def opSmallStruct(self, p1, current=None):
        return (p1, p1)

    def opFixedStruct(self, p1, current=None):
        return (p1, p1)

    def opVarStruct(self, p1, current=None):
        return (p1, p1)

    def opOneOptional(self, p1, current=None):
        return (p1, p1)

    def opOneOptionalProxy(self, p1, current=None):
        return (p1, p1)

    def opByteSeq(self, p1, current=None):
        return (p1, p1)

    def opBoolSeq(self, p1, current=None):
        return (p1, p1)

    def opShortSeq(self, p1, current=None):
        return (p1, p1)

    def opIntSeq(self, p1, current=None):
        return (p1, p1)

    def opLongSeq(self, p1, current=None):
        return (p1, p1)

    def opFloatSeq(self, p1, current=None):
        return (p1, p1)

    def opDoubleSeq(self, p1, current=None):
        return (p1, p1)

    def opStringSeq(self, p1, current=None):
        return (p1, p1)

    def opSmallStructSeq(self, p1, current=None):
        return (p1, p1)

    def opSmallStructList(self, p1, current=None):
        return (p1, p1)

    def opFixedStructSeq(self, p1, current=None):
        return (p1, p1)

    def opFixedStructList(self, p1, current=None):
        return (p1, p1)

    def opVarStructSeq(self, p1, current=None):
        return (p1, p1)

    def opSerializable(self, p1, current=None):
        return (p1, p1)

    def opIntIntDict(self, p1, current=None):
        return (p1, p1)

    def opStringIntDict(self, p1, current=None):
        return (p1, p1)

    def opIntOneOptionalDict(self, p1, current=None):
        return (p1, p1)

    def opClassAndUnknownOptional(self, p, current=None):
        pass

    def sendOptionalClass(self, req, o, current=None):
        pass

    def returnOptionalClass(self, req, current=None):
        return Test.OneOptional(53)

    def opG(self, g, current=None):
        return g

    def opVoid(self, current=None):
        pass

    def opMStruct1(self, current):
        return Test.Initial.OpMStruct1MarshaledResult(Test.SmallStruct(), current)

    def opMStruct2(self, p1, current):
        return Test.Initial.OpMStruct2MarshaledResult((p1, p1), current)

    def opMSeq1(self, current):
        return Test.Initial.OpMSeq1MarshaledResult([], current)

    def opMSeq2(self, p1, current):
        return Test.Initial.OpMSeq2MarshaledResult((p1, p1), current)

    def opMDict1(self, current):
        return Test.Initial.OpMDict1MarshaledResult({}, current)

    def opMDict2(self, p1, current):
        return Test.Initial.OpMDict2MarshaledResult((p1, p1), current)

    def opMG1(self, current):
        return Test.Initial.OpMG1MarshaledResult(Test.G(), current)

    def opMG2(self, p1, current):
        return Test.Initial.OpMG2MarshaledResult((p1, p1), current)

    def supportsRequiredParams(self, current=None):
        return False

    def supportsJavaSerializable(self, current=None):
        return True

    def supportsCsharpSerializable(self, current=None):
        return True

    def supportsCppStringView(self, current=None):
        return False

    def supportsNullOptional(self, current=None):
        return True


class Server(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            initial = InitialI()
            adapter.add(initial, Ice.stringToIdentity("initial"))
            adapter.activate()

            communicator.waitForShutdown()
