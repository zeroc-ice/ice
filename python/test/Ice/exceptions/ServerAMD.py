#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, array

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice('"-I' + slice_dir + '" TestAMD.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class ThrowerI(Test.Thrower):
    def shutdown_async(self, cb, current=None):
        current.adapter.getCommunicator().shutdown()
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
        # Python-specific: make sure the argument is validated.
        try:
            cb.ice_exception('foo')
            test(False)
        except TypeError:
            pass

        cb.ice_exception(RuntimeError("12345"))

    def throwAssertException_async(self, cb, current=None):
        raise RuntimeError("operation `throwAssertException' not supported")

    def throwMemoryLimitException_async(self, cb, seq, current=None):
        cb.ice_response(bytearray(20 * 1024))

    def throwLocalExceptionIdempotent_async(self, cb, current=None):
        cb.ice_exception(Ice.TimeoutException())

    def throwAfterResponse_async(self, cb, current=None):
        cb.ice_response()
        raise RuntimeError("12345")

    def throwAfterException_async(self, cb, current=None):
        cb.ice_exception(Test.A())
        raise RuntimeError("12345")

def run(args, communicator):
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter2 = communicator.createObjectAdapter("TestAdapter2")
    adapter3 = communicator.createObjectAdapter("TestAdapter3")
    object = ThrowerI()
    adapter.add(object, communicator.stringToIdentity("thrower"))
    adapter2.add(object, communicator.stringToIdentity("thrower"))
    adapter3.add(object, communicator.stringToIdentity("thrower"))
    adapter.activate()
    adapter2.activate()
    adapter3.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Dispatch", "0")
    initData.properties.setProperty("Ice.Warn.Connections", "0");
    initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010:udp")
    initData.properties.setProperty("Ice.MessageSizeMax", "10")
    initData.properties.setProperty("TestAdapter2.Endpoints", "default -p 12011")
    initData.properties.setProperty("TestAdapter2.MessageSizeMax", "0")
    initData.properties.setProperty("TestAdapter3.Endpoints", "default -p 12012")
    initData.properties.setProperty("TestAdapter3.MessageSizeMax", "1")
    communicator = Ice.initialize(sys.argv, initData)
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
