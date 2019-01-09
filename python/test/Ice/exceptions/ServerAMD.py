#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Ice
import Test


class ThrowerI(Test.Thrower):
    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def supportsUndeclaredExceptions(self, current=None):
        return True

    def supportsAssertException(self, current=None):
        return False

    def throwAasA(self, a, current=None):
        ex = Test.A()
        ex.aMem = a
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwAorDasAorD(self, a, current=None):
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

    def throwBasA(self, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasA(self, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwBasB(self, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasB(self, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwCasC(self, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwModA(self, a, a2, current=None):
        ex = Test.Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA(self, a, current=None):
        ex = Test.A()
        ex.aMem = a
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwUndeclaredB(self, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwUndeclaredC(self, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    def throwLocalException(self, current=None):
        f = Ice.Future()
        f.set_exception(Ice.TimeoutException())
        return f

    def throwNonIceException(self, current=None):
        f = Ice.Future()
        f.set_exception(RuntimeError("12345"))
        return f

    def throwAssertException(self, current=None):
        raise RuntimeError("operation `throwAssertException' not supported")

    def throwMemoryLimitException(self, seq, current=None):
        return Ice.Future.completed(bytearray(20 * 1024))

    def throwLocalExceptionIdempotent(self, current=None):
        f = Ice.Future()
        f.set_exception(Ice.TimeoutException())
        return f

    def throwAfterResponse(self, current=None):
        # Cannot be implemented with Futures
        return None

    def throwAfterException(self, current=None):
        # Cannot be implemented with Futures
        f = Ice.Future()
        f.set_exception(Test.A())
        return f


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
