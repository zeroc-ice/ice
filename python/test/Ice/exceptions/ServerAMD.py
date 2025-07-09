#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import Test

import Ice
from TestHelper import TestHelper


class ThrowerI(Test.Thrower):
    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

    def supportsUndeclaredExceptions(self, current):
        return True

    def supportsAssertException(self, current):
        return False

    def throwAasA(self, a, current):
        ex = Test.A()
        ex.aMem = a
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwAorDasAorD(self, a, current):
        f = Ice.Future()
        if a > 0:
            ex = Test.A()
            ex.aMem = a
            f.set_exception(ex)
        else:
            ex = Test.D()
            ex.dMem = a
            f.set_exception(ex)
        return f

    def throwBasA(self, a, b, current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasA(self, a, b, c, current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwBasB(self, a, b, current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasB(self, a, b, c, current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwCasC(self, a, b, c, current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwModA(self, a, a2, current):
        ex = Test.Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA(self, a, current):
        ex = Test.A()
        ex.aMem = a
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwUndeclaredB(self, a, b, current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwUndeclaredC(self, a, b, c, current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwLocalException(self, current):
        f = Ice.Future()
        f.set_exception(Ice.TimeoutException())
        return f

    def throwNonIceException(self, current):
        f = Ice.Future()
        f.set_exception(RuntimeError("12345"))
        return f

    def throwAssertException(self, current):
        raise RuntimeError("operation `throwAssertException' not supported")

    def throwMemoryLimitException(self, seq, current):
        return Ice.Future.completed(bytearray(20 * 1024))

    def throwLocalExceptionIdempotent(self, current):
        f = Ice.Future()
        f.set_exception(Ice.TimeoutException())
        return f

    def throwDispatchException(self, replyStatus, current):
        f = Ice.Future()
        f.set_exception(Ice.DispatchException(replyStatus))
        return f

    def throwAfterResponse(self, current):
        # Cannot be implemented with Futures
        return None

    def throwAfterException(self, current):
        # Cannot be implemented with Futures
        f = Ice.Future()
        f.set_exception(Test.A())
        return f

    def throwMarshalException(self, current):
        if "return" in current.ctx:
            return Ice.Future.completed(("", 0))
        if "param" in current.ctx:
            return Ice.Future.completed((0, ""))
        return Ice.Future.completed(None)

    def throwRequestFailedException(self, typeName, id, facet, operation, current):
        if typeName == "Ice.ObjectNotExistException":
            raise Ice.ObjectNotExistException(id, facet, operation)
        elif typeName == "Ice.FacetNotExistException":
            raise Ice.FacetNotExistException(id, facet, operation)
        elif typeName == "Ice.OperationNotExistException":
            raise Ice.OperationNotExistException(id, facet, operation)


class ServerAMD(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        properties.setProperty("Ice.Warn.Connections", "0")
        properties.setProperty("Ice.MessageSizeMax", "10")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", self.getTestEndpoint(num=1))
            communicator.getProperties().setProperty("TestAdapter2.MessageSizeMax", "0")
            communicator.getProperties().setProperty("TestAdapter3.Endpoints", self.getTestEndpoint(num=2))
            communicator.getProperties().setProperty("TestAdapter3.MessageSizeMax", "1")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter2 = communicator.createObjectAdapter("TestAdapter2")
            adapter3 = communicator.createObjectAdapter("TestAdapter3")
            object = ThrowerI()
            adapter.add(object, Ice.stringToIdentity("thrower"))
            adapter2.add(object, Ice.stringToIdentity("thrower"))
            adapter3.add(object, Ice.stringToIdentity("thrower"))
            adapter.activate()
            adapter2.activate()
            adapter3.activate()
            communicator.waitForShutdown()
