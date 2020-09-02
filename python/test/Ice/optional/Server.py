#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Ice
import Test


class InitialI(Test.Initial):

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def pingPong(self, o, current=None):
        return o

    def opOptionalException(self, a, b, vs, current=None):
        raise Test.OptionalException(False, a, b, vs)

    def opDerivedException(self, a, b, vs, current=None):
        raise Test.DerivedException(False, a, b, vs, b, vs)

    def opRequiredException(self, a, b, vs, current=None):
        e = Test.RequiredException()
        e.a = a
        e.b = b
        e.vs = vs
        if b is not Ice.Unset:
            e.ss = b
        if vs is not Ice.Unset:
            e.vs2 = vs
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

    def opClassAndUnknownOptional(self, p, current=None):
        pass

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
