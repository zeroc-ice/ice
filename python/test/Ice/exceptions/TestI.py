# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, array, sys

class ThrowerI(Test._ThrowerDisp):
    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def supportsUndeclaredExceptions(self, current=None):
        return True

    def supportsAssertException(self, current=None):
        return False

    def throwAasA(self, a, current=None):
        ex = Test.A()
        ex.aMem = a
        raise ex

    def throwAorDasAorD(self, a, current=None):
        if a > 0:
            ex = Test.A()
            ex.aMem = a
            raise ex
        else:
            ex = Test.D()
            ex.dMem = a
            raise ex

    def throwBasA(self, a, b, current=None):
        self.throwBasB(a, b, current)

    def throwCasA(self, a, b, c, current=None):
        self.throwCasC(a, b, c, current)

    def throwBasB(self, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasB(self, a, b, c, current=None):
        self.throwCasC(a, b, c, current)

    def throwCasC(self, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    def throwModA(self, a, a2, current=None):
        ex = Test.Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA(self, a, current=None):
        ex = Test.A()
        ex.aMem = a
        raise ex

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
        raise ex

    def throwLocalException(self, current=None):
        raise Ice.TimeoutException()

    def throwNonIceException(self, current=None):
        raise RuntimeError("12345")

    def throwAssertException(self, current=None):
        raise RuntimeError("operation `throwAssertException' not supported")

    def throwMemoryLimitException(self, seq, current=None):
        return bytearray(20 * 1024)

    def throwLocalExceptionIdempotent(self, current=None):
        raise Ice.TimeoutException()

    def throwAfterResponse(self, current=None):
        #
        # Only relevant for AMD.
        #
        pass

    def throwAfterException(self, current=None):
        #
        # Only relevant for AMD.
        #
        raise Test.A()
