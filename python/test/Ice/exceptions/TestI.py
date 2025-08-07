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
        f = Ice.Future()
        f.set_exception(ex)
        return f

    @override
    def throwAorDasAorD(self, a: int, current: Ice.Current):
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

    @override
    def throwBasA(self, a: int, b: int, current: Ice.Current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    @override
    def throwCasA(self, a: int, b: int, c: int, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    @override
    def throwBasB(self, a: int, b: int, current: Ice.Current):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    @override
    def throwCasB(self, a: int, b: int, c: int, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

    @override
    def throwCasC(self, a: int, b: int, c: int, current: Ice.Current):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        f = Ice.Future()
        f.set_exception(ex)
        return f

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
        f = Ice.Future()
        f.set_exception(ex)
        return f

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
        f = Ice.Future()
        f.set_exception(ex)
        return f

    @override
    def throwLocalException(self, current: Ice.Current):
        f = Ice.Future()
        f.set_exception(Ice.TimeoutException())
        return f

    @override
    def throwNonIceException(self, current: Ice.Current):
        f = Ice.Future()
        f.set_exception(RuntimeError("12345"))
        return f

    @override
    def throwAssertException(self, current: Ice.Current):
        raise RuntimeError("operation `throwAssertException' not supported")

    @override
    def throwMemoryLimitException(self, seq: bytes, current: Ice.Current):
        return Ice.Future.completed(bytearray(20 * 1024))

    @override
    def throwLocalExceptionIdempotent(self, current: Ice.Current):
        f = Ice.Future()
        f.set_exception(Ice.TimeoutException())
        return f

    @override
    def throwDispatchException(self, replyStatus: int, current: Ice.Current):
        f = Ice.Future()
        f.set_exception(Ice.DispatchException(replyStatus))
        return f

    @override
    def throwAfterResponse(self, current: Ice.Current):
        # Cannot be implemented with Futures
        return None

    @override
    def throwAfterException(self, current: Ice.Current):
        # Cannot be implemented with Futures
        f = Ice.Future()
        f.set_exception(Test.A())
        return f

    @override
    def throwMarshalException(self, current: Ice.Current):
        if "return" in current.ctx:
            return Ice.Future.completed(("", 0))
        if "param" in current.ctx:
            return Ice.Future.completed((0, ""))
        return Ice.Future.completed(None)

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
