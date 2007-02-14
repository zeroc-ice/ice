#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.insert(0, os.path.join(toplevel, "python"))
sys.path.insert(0, os.path.join(toplevel, "lib"))

import Ice
Ice.loadSlice('TestAMD.ice')
import Test

class ThrowerI(Test.Thrower):
    def __init__(self, adapter):
        self._adapter = adapter

    def shutdown_async(self, cb, current=None):
        self._adapter.getCommunicator().shutdown()
        cb.ice_response()

    def supportsUndeclaredExceptions_async(self, cb, current=None):
        cb.ice_response(True)

    def supportsAssertException_async(self, cb, current=None):
        cb.ice_response(False)

    def throwAasA_async(self, cb, a, current=None):
        ex = Test.A()
        ex.aMem = a
        cb.ice_exception(ex)

    def throwAorDasAorD_async(self, cb, a, current=None):
        if a > 0:
            ex = Test.A()
            ex.aMem = a
            cb.ice_exception(ex)
        else:
            ex = Test.D()
            ex.dMem = a
            cb.ice_exception(ex)

    def throwBasA_async(self, cb, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex
        #cb.ice_exception(ex)

    def throwCasA_async(self, cb, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        cb.ice_exception(ex)

    def throwBasB_async(self, cb, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex
        #cb.ice_exception(ex)

    def throwCasB_async(self, cb, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        cb.ice_exception(ex)

    def throwCasC_async(self, cb, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        cb.ice_exception(ex)

    def throwModA_async(self, cb, a, a2, current=None):
        ex = Test.Mod.A()
        ex.aMem = a
        ex.a2Mem = a2
        raise ex

    def throwUndeclaredA_async(self, cb, a, current=None):
        ex = Test.A()
        ex.aMem = a
        cb.ice_exception(ex)

    def throwUndeclaredB_async(self, cb, a, b, current=None):
        ex = Test.B()
        ex.aMem = a
        ex.bMem = b
        raise ex
        #cb.ice_exception(ex)

    def throwUndeclaredC_async(self, cb, a, b, c, current=None):
        ex = Test.C()
        ex.aMem = a
        ex.bMem = b
        ex.cMem = c
        cb.ice_exception(ex)

    def throwLocalException_async(self, cb, current=None):
        cb.ice_exception(Ice.TimeoutException())

    def throwNonIceException_async(self, cb, current=None):
        cb.ice_exception(RuntimeError("12345"))

    def throwAssertException_async(self, cb, current=None):
        raise RuntimeError("operation `throwAssertException' not supported")

def run(args, communicator):
    properties = communicator.getProperties()
    properties.setProperty("Ice.Warn.Dispatch", "0")
    properties.setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = ThrowerI(adapter)
    adapter.add(object, communicator.stringToIdentity("thrower"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    communicator = Ice.initialize(sys.argv)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
