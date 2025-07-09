#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import Test
from TestHelper import TestHelper

import Ice


class InitialI(Test.Initial):
    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

    def pingPong(self, o, current):
        return o

    def opOptionalException(self, a, b, current):
        raise Test.OptionalException(False, a, b)

    def opDerivedException(self, a, b, current):
        raise Test.DerivedException(False, a, b, "d1", b, "d2")

    def opRequiredException(self, a, b, current):
        e = Test.RequiredException()
        e.a = a
        e.b = b
        if b is not None:
            e.ss = b
        raise e

    def opByte(self, p1, current):
        return (p1, p1)

    def opBool(self, p1, current):
        return (p1, p1)

    def opShort(self, p1, current):
        return (p1, p1)

    def opInt(self, p1, current):
        return (p1, p1)

    def opLong(self, p1, current):
        return (p1, p1)

    def opFloat(self, p1, current):
        return (p1, p1)

    def opDouble(self, p1, current):
        return (p1, p1)

    def opString(self, p1, current):
        return (p1, p1)

    def opMyEnum(self, p1, current):
        return (p1, p1)

    def opSmallStruct(self, p1, current):
        return (p1, p1)

    def opFixedStruct(self, p1, current):
        return (p1, p1)

    def opVarStruct(self, p1, current):
        return (p1, p1)

    def opOneOptional(self, p1, current):
        return (p1, p1)

    def opMyInterfaceProxy(self, p1, current):
        return (p1, p1)

    def opByteSeq(self, p1, current):
        return (p1, p1)

    def opBoolSeq(self, p1, current):
        return (p1, p1)

    def opShortSeq(self, p1, current):
        return (p1, p1)

    def opIntSeq(self, p1, current):
        return (p1, p1)

    def opLongSeq(self, p1, current):
        return (p1, p1)

    def opFloatSeq(self, p1, current):
        return (p1, p1)

    def opDoubleSeq(self, p1, current):
        return (p1, p1)

    def opStringSeq(self, p1, current):
        return (p1, p1)

    def opSmallStructSeq(self, p1, current):
        return (p1, p1)

    def opSmallStructList(self, p1, current):
        return (p1, p1)

    def opFixedStructSeq(self, p1, current):
        return (p1, p1)

    def opFixedStructList(self, p1, current):
        return (p1, p1)

    def opVarStructSeq(self, p1, current):
        return (p1, p1)

    def opSerializable(self, p1, current):
        return (p1, p1)

    def opIntIntDict(self, p1, current):
        return (p1, p1)

    def opStringIntDict(self, p1, current):
        return (p1, p1)

    def opClassAndUnknownOptional(self, p, current):
        pass

    def opG(self, g, current):
        return g

    def opVoid(self, current):
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

    def opRequiredAfterOptional(self, p1, p2, p3, current):
        return (p1, p2, p3)

    def opOptionalAfterRequired(self, p1, p2, p3, current):
        return (p1, p2, p3)

    def supportsJavaSerializable(self, current):
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
