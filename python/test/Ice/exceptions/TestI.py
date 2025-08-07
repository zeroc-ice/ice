# Copyright (c) ZeroC, Inc.

from typing import override

from generated.test.Ice.exceptions import Test
from generated.test.Ice.exceptions.Test import Mod

import Ice


class ThrowerI(Test.Thrower):
    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    @override
    def supportsUndeclaredExceptions(self, current: Ice.Current):
        return True

    @override
    def supportsAssertException(self, current: Ice.Current):
        return False

    @override
    def throwAasA(self, a: int, current: Ice.Current):
        ex = Test.A()
        ex.aMem = a
        raise ex

    @override
    def throwAorDasAorD(self, a: int, current: Ice.Current):
        if a > 0:
            ex = Test.A()
            ex.aMem = a
            raise ex
        else:
            ex = Test.D()
            ex.dMem = a
            raise ex

    @override
    def throwBasA(self, a: int, b: int, current: Ice.Current):
        self.throwBasB(a, b, current)

    @override
    def throwCasA(self, a: int, b: int, c: int, current: Ice.Current):
        self.throwCasC(a, b, c, current)

    def throwBasB(self, a: int, b: int, current: Ice.Current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    @override
    def throwCasB(self, a: int, b: int, c: int, current: Ice.Current):
        self.throwCasC(a, b, c, current)

    @override
    def throwCasC(self, a: int, b: int, c: int, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    @override
    def throwModA(self, a: int, a2: int, current: Ice.Current):
        ex = Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    @override
    def throwUndeclaredA(self, a: int, current: Ice.Current):
        ex = Test.A()
        ex.aMem = a
        raise ex

    @override
    def throwUndeclaredB(self, a: int, b: int, current: Ice.Current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    @override
    def throwUndeclaredC(self, a: int, b: int, c: int, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    @override
    def throwLocalException(self, current: Ice.Current):
        raise Ice.TimeoutException()

    @override
    def throwNonIceException(self, current: Ice.Current):
        raise RuntimeError("12345")

    @override
    def throwAssertException(self, current: Ice.Current):
        raise RuntimeError("operation 'throwAssertException' not supported")

    @override
    def throwMemoryLimitException(self, seq: bytes, current: Ice.Current):
        return bytearray(20 * 1024)

    @override
    def throwLocalExceptionIdempotent(self, current: Ice.Current):
        raise Ice.TimeoutException()

    @override
    def throwDispatchException(self, replyStatus: int, current: Ice.Current):
        raise Ice.DispatchException(replyStatus)

    @override
    def throwAfterResponse(self, current: Ice.Current):
        #
        # Only relevant for AMD.
        #
        pass

    @override
    def throwAfterException(self, current: Ice.Current):
        #
        # Only relevant for AMD.
        #
        raise Test.A()

    @override
    def throwMarshalException(self, current: Ice.Current) -> tuple[int, int]:
        # We return the wrong type on purpose to test that we throw MarshalException.
        if "return" in current.ctx:
            return ("", 0)  # type: ignore
        if "param" in current.ctx:
            return (0, "")  # type: ignore
        return None  # type: ignore

    @override
    def throwRequestFailedException(
        self, type: str, id: Ice.Identity, facet: str, operation: str, current: Ice.Current
    ):
        if type == "Ice.ObjectNotExistException":
            raise Ice.ObjectNotExistException(id, facet, operation)
        elif type == "Ice.FacetNotExistException":
            raise Ice.FacetNotExistException(id, facet, operation)
        elif type == "Ice.OperationNotExistException":
            raise Ice.OperationNotExistException(id, facet, operation)
