# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, Test, Mod, _Top

class ThrowerI(_Top.Thrower):
    def __init__(self, adapter):
        self._adapter = adapter

    def shutdown(self, current=None):
        self._adapter.getCommunicator().shutdown()

    def supportsUndeclaredExceptions(self, current=None):
        return True

    def supportsAssertException(self, current=None):
        return False

    def throwAasA(self, a, current=None):
        ex = _Top.A()
        ex.aMem = a
        raise ex

    def throwAorDasAorD(self, a, current=None):
        if a > 0:
            ex = _Top.A()
            ex.aMem = a
            raise ex
        else:
            ex = _Top.D()
            ex.dMem = a
            raise ex

    def throwBasA(self, a, b, current=None):
        self.throwBasB(a, b, current)

    def throwCasA(self, a, b, c, current=None):
        self.throwCasC(a, b, c, current)

    def throwBasB(self, a, b, current=None):
        ex = _Top.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwCasB(self, a, b, c, current=None):
        self.throwCasC(a, b, c, current)

    def throwCasC(self, a, b, c, current=None):
        ex = _Top.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        raise ex

    def throwModA(self, a, a2, current=None):
        ex = Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA(self, a, current=None):
        ex = _Top.A()
        ex.aMem = a
        raise ex

    def throwUndeclaredB(self, a, b, current=None):
        ex = _Top.B()
        ex.aMem = a
        ex.bMem = b
        raise ex

    def throwUndeclaredC(self, a, b, c, current=None):
        ex = _Top.C()
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

def run(args, communicator):
    properties = communicator.getProperties()
    properties.setProperty("Ice.Warn.Dispatch", "0")
    properties.setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = ThrowerI(adapter)
    adapter.add(object, Ice.stringToIdentity("thrower"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    communicator = Ice.initialize(sys.argv)
    status = run(sys.argv, communicator)
except Ice.Exception, ex:
    print ex
    status = False

if communicator:
    try:
        communicator.destroy()
    except Ice.Exception, ex:
        print ex
        status = False

sys.exit(not status)
