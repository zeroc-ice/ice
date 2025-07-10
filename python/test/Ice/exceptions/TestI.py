# Copyright (c) ZeroC, Inc.

import sys
import Test

import Ice


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
        raise ex

    def throwAorDasAorD(self, a, current):
        if a > 0:
            ex = Test.A()
            ex.aMem = a
            raise ex
        else:
            ex = Test.D()
            ex.dMem = a
            raise ex

    def throwBasA(self, a, b, current):
        self.throwBasB(a, b, current)

    def throwCasA(self, a, b, c, current):
        self.throwCasC(a, b, c, current)

    def throwBasB(self, a, b, current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasB(self, a, b, c, current):
        self.throwCasC(a, b, c, current)

    def throwCasC(self, a, b, c, current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    def throwModA(self, a, a2, current):
        ex = Test.Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA(self, a, current):
        ex = Test.A()
        ex.aMem = a
        raise ex

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
        raise ex

    def throwLocalException(self, current):
        raise Ice.TimeoutException()

    def throwNonIceException(self, current):
        raise RuntimeError("12345")

    def throwAssertException(self, current):
        raise RuntimeError("operation 'throwAssertException' not supported")

    def throwMemoryLimitException(self, seq, current):
        return bytearray(20 * 1024)

    def throwLocalExceptionIdempotent(self, current):
        raise Ice.TimeoutException()

    def throwDispatchException(self, replyStatus, current):
        ex = Ice.DispatchException(replyStatus)

        print("replyStatus of Ice.DispatchException: ", replyStatus, file=sys.stderr)
        print("typeof replyStatus", type(replyStatus), file=sys.stderr)

        raise Ice.DispatchException(replyStatus)

    def throwAfterResponse(self, current):
        #
        # Only relevant for AMD.
        #
        pass

    def throwAfterException(self, current):
        #
        # Only relevant for AMD.
        #
        raise Test.A()

    def throwMarshalException(self, current):
        if "return" in current.ctx:
            return ("", 0)
        if "param" in current.ctx:
            return (0, "")

    def throwRequestFailedException(self, typeName, id, facet, operation, current):
        if typeName == "Ice.ObjectNotExistException":
            raise Ice.ObjectNotExistException(id, facet, operation)
        elif typeName == "Ice.FacetNotExistException":
            raise Ice.FacetNotExistException(id, facet, operation)
        elif typeName == "Ice.OperationNotExistException":
            raise Ice.OperationNotExistException(id, facet, operation)
