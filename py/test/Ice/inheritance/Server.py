#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
Ice.loadSlice('Test.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CAI(Test.MA.CA):
    def caop(self, p, current=None):
        return p

class CBI(Test.MB.CB, CAI):
     def cbop(self, p, current=None):
        return p

class CCI(Test.MA.CC, CBI):
    def ccop(self, p, current=None):
        return p

class IAI(Test.MA.IA):
    def iaop(self, p, current=None):
        return p

class IB1I(Test.MB.IB1, IAI):
    def ib1op(self, p, current=None):
        return p

class IB2I(Test.MB.IB2, IAI):
    def ib2op(self, p, current=None):
        return p

class ICI(Test.MA.IC, IB1I, IB2I):
    def icop(self, p, current=None):
        return p

class CDI(Test.MA.CD, CCI, IB1I, IB2I):
    def cdop(self, p, current=None):
        return p

class InitialI(Test.Initial):
    def __init__(self, adapter):
        self._adapter = adapter
        self._ca = Test.MA.CAPrx.uncheckedCast(self._adapter.addWithUUID(CAI()))
        self._cb = Test.MB.CBPrx.uncheckedCast(self._adapter.addWithUUID(CBI()))
        self._cc = Test.MA.CCPrx.uncheckedCast(self._adapter.addWithUUID(CCI()))
        self._cd = Test.MA.CDPrx.uncheckedCast(self._adapter.addWithUUID(CDI()))
        self._ia = Test.MA.IAPrx.uncheckedCast(self._adapter.addWithUUID(IAI()))
        self._ib1 = Test.MB.IB1Prx.uncheckedCast(self._adapter.addWithUUID(IB1I()))
        self._ib2 = Test.MB.IB2Prx.uncheckedCast(self._adapter.addWithUUID(IB2I()))
        self._ic = Test.MA.ICPrx.uncheckedCast(self._adapter.addWithUUID(ICI()))

    def shutdown(self, current=None):
        self._adapter.getCommunicator().shutdown()

    def caop(self, current=None):
        return self._ca

    def cbop(self, current=None):
        return self._cb

    def ccop(self, current=None):
        return self._cc

    def cdop(self, current=None):
        return self._cd

    def iaop(self, current=None):
        return self._ia

    def ib1op(self, current=None):
        return self._ib1

    def ib2op(self, current=None):
        return self._ib2

    def icop(self, current=None):
        return self._ic

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = InitialI(adapter)
    adapter.add(object, Ice.stringToIdentity("initial"))
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
