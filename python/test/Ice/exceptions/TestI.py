# Copyright (c) ZeroC, Inc.

from generated.test.Ice.exceptions import Test

import Ice


class ThrowerI(Test.Thrower):
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    def supportsUndeclaredExceptions(self, current: Ice.Current):
        return True

    def supportsAssertException(self, current: Ice.Current):
        return False

    def throwAasA(self, a, current: Ice.Current):
        ex = Test.A()
        ex.aMem = a
        raise ex

    def throwAorDasAorD(self, a, current: Ice.Current):
        if a > 0:
            ex = Test.A()
            ex.aMem = a
            raise ex
        else:
            ex = Test.D()
            ex.dMem = a
            raise ex

    def throwBasA(self, a, b, current: Ice.Current):
        self.throwBasB(a, b, current)

    def throwCasA(self, a, b, c, current: Ice.Current):
        self.throwCasC(a, b, c, current)

    def throwBasB(self, a, b, current: Ice.Current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasB(self, a, b, c, current: Ice.Current):
        self.throwCasC(a, b, c, current)

    def throwCasC(self, a, b, c, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    def throwModA(self, a, a2, current: Ice.Current):
        ex = Test.Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA(self, a, current: Ice.Current):
        ex = Test.A()
        ex.aMem = a
        raise ex

    def throwUndeclaredB(self, a, b, current: Ice.Current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwUndeclaredC(self, a, b, c, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    def throwLocalException(self, current: Ice.Current):
        raise Ice.TimeoutException()

    def throwNonIceException(self, current: Ice.Current):
        raise RuntimeError("12345")

    def throwAssertException(self, current: Ice.Current):
        raise RuntimeError("operation 'throwAssertException' not supported")

    def throwMemoryLimitException(self, seq, current: Ice.Current):
        return bytearray(20 * 1024)

    def throwLocalExceptionIdempotent(self, current: Ice.Current):
        raise Ice.TimeoutException()

    def throwDispatchException(self, replyStatus, current: Ice.Current):
        raise Ice.DispatchException(replyStatus)

    def throwAfterResponse(self, current: Ice.Current):
        #
        # Only relevant for AMD.
        #
        pass

    def throwAfterException(self, current: Ice.Current):
        #
        # Only relevant for AMD.
        #
        raise Test.A()

    def throwMarshalException(self, current: Ice.Current):
        if "return" in current.ctx:
            return ("", 0)
        if "param" in current.ctx:
            return (0, "")

    def throwRequestFailedException(self, typeName, id, facet, operation, current: Ice.Current):
        if typeName == "Ice.ObjectNotExistException":
            raise Ice.ObjectNotExistException(id, facet, operation)
        elif typeName == "Ice.FacetNotExistException":
            raise Ice.FacetNotExistException(id, facet, operation)
        elif typeName == "Ice.OperationNotExistException":
            raise Ice.OperationNotExistException(id, facet, operation)
